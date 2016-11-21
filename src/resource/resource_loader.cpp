/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "config.h"
#include "dynamic_string.h"
#include "filesystem.h"
#include "memory.h"
#include "os.h"
#include "path.h"
#include "queue.h"
#include "resource_loader.h"
#include "temp_allocator.h"

namespace crown
{
ResourceLoader::ResourceLoader(Filesystem& fs)
	: _fs(fs)
	, _requests(default_allocator())
	, _loaded(default_allocator())
	, _exit(false)
{
	_thread.start(ResourceLoader::thread_proc, this);
}

ResourceLoader::~ResourceLoader()
{
	_exit = true;
	_thread.stop();
}

bool ResourceLoader::can_load(StringId64 type, StringId64 name)
{
	TempAllocator128 ta;
	DynamicString type_str(ta);
	DynamicString name_str(ta);
	type.to_string(type_str);
	name.to_string(name_str);

	DynamicString res_path(ta);
	res_path += type_str;
	res_path += '-';
	res_path += name_str;

	DynamicString path(ta);
	path::join(CROWN_DATA_DIRECTORY, res_path.c_str(), path);

	return _fs.exists(path.c_str());
}

void ResourceLoader::add_request(const ResourceRequest& rr)
{
	ScopedMutex sm(_mutex);
	queue::push_back(_requests, rr);
}

void ResourceLoader::flush()
{
	while (num_requests()) {}
}

u32 ResourceLoader::num_requests()
{
	ScopedMutex sm(_mutex);
	return queue::size(_requests);
}

void ResourceLoader::add_loaded(ResourceRequest rr)
{
	ScopedMutex sm(_loaded_mutex);
	queue::push_back(_loaded, rr);
}

void ResourceLoader::get_loaded(Array<ResourceRequest>& loaded)
{
	ScopedMutex sm(_loaded_mutex);

	const u32 num = queue::size(_loaded);
	array::reserve(loaded, num);

	for (u32 i = 0; i < num; ++i)
	{
		array::push_back(loaded, queue::front(_loaded));
		queue::pop_front(_loaded);
	}
}

s32 ResourceLoader::run()
{
	while (!_exit)
	{
		_mutex.lock();
		if (queue::empty(_requests))
		{
			_mutex.unlock();
			os::sleep(16);
			continue;
		}

		ResourceRequest rr = queue::front(_requests);
		_mutex.unlock();

		TempAllocator128 ta;
		DynamicString type_str(ta);
		DynamicString name_str(ta);
		rr.type.to_string(type_str);
		rr.name.to_string(name_str);

		DynamicString res_path(ta);
		res_path += type_str;
		res_path += '-';
		res_path += name_str;

		DynamicString path(ta);
		path::join(CROWN_DATA_DIRECTORY, res_path.c_str(), path);

		File* file = _fs.open(path.c_str(), FileOpenMode::READ);
		rr.data = rr.load_function(*file, *rr.allocator);
		_fs.close(*file);

		add_loaded(rr);
		_mutex.lock();
		queue::pop_front(_requests);
		_mutex.unlock();
	}

	return 0;
}

s32 ResourceLoader::thread_proc(void* thiz)
{
	return ((ResourceLoader*)thiz)->run();
}

} // namespace crown
