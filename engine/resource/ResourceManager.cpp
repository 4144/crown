/*
Copyright (c) 2013 Daniele Bartolini, Michele Rossi
Copyright (c) 2012 Daniele Bartolini, Simone Boscaratto

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
*/

#include <algorithm>
#include <cstdio>

#include "Types.h"
#include "ResourceManager.h"
#include "ResourceRegistry.h"
#include "Allocator.h"
#include "StringUtils.h"
#include "Hash.h"
#include "Path.h"
#include "Device.h"
#include "Filesystem.h"
#include "TextReader.h"
#include "DiskFile.h"
#include "TempAllocator.h"
#include "List.h"

namespace crown
{

//-----------------------------------------------------------------------------
ResourceManager::ResourceManager(Bundle& bundle) :
	m_resource_heap("resource", default_allocator()),
	m_loader(bundle, m_resource_heap),
	m_seed(0),
	m_resources(default_allocator())
{
	DiskFile* seed_file = device()->filesystem()->open("seed.ini", FOM_READ);
	TextReader reader(*seed_file);

	char tmp_buf[32];
	reader.read_string(tmp_buf, 32);

	device()->filesystem()->close(seed_file);

	sscanf(tmp_buf, "%u", &m_seed);

	m_loader.start();
}

//-----------------------------------------------------------------------------
ResourceManager::~ResourceManager()
{
	m_loader.stop();
}

//-----------------------------------------------------------------------------
ResourceId ResourceManager::load(const char* name)
{
	char basename[512];
	char extension[512];

	path::filename_without_extension(name, basename, 512);
	path::extension(name, extension, 512);

	uint32_t name_hash = hash::murmur2_32(basename, string::strlen(basename), m_seed);
	uint32_t type_hash = hash::murmur2_32(extension, string::strlen(extension), 0);

	return load(name_hash, type_hash);
}

//-----------------------------------------------------------------------------
void ResourceManager::unload(ResourceId name)
{
	CE_ASSERT(has(name), "Resource not loaded: %.8X%.8X", name.name, name.type);

	ResourceEntry& entry = m_resources[name.index];
	
	entry.references--;
	
	if (entry.references == 0 && entry.state == RS_LOADED)
	{
		resource_on_unload(name.type, m_resource_heap, entry.resource);

		entry.state = RS_UNLOADED;
		entry.resource = NULL;
	}
}

//-----------------------------------------------------------------------------
bool ResourceManager::has(ResourceId name) const
{
	if (m_resources.size() > name.index)
	{
		 return (m_resources[name.index].id.name == name.name);
	}

	return false;
}

//-----------------------------------------------------------------------------
const void* ResourceManager::data(ResourceId name) const
{
	CE_ASSERT(has(name), "Resource not loaded: %.8X%.8X", name.name, name.type);

	return m_resources[name.index].resource;
}

//-----------------------------------------------------------------------------
bool ResourceManager::is_loaded(ResourceId name) const
{
	CE_ASSERT(has(name), "Resource not loaded: %.8X%.8X", name.name, name.type);

	return m_resources[name.index].state == RS_LOADED;
}

//-----------------------------------------------------------------------------
uint32_t ResourceManager::references(ResourceId name) const
{
	CE_ASSERT(has(name), "Resource not loaded: %.8X%.8X", name.name, name.type);

	return m_resources[name.index].references;
}

//-----------------------------------------------------------------------------
void ResourceManager::flush()
{
	while (m_loader.remaining() > 0) ;

	poll_resource_loader();
}

//-----------------------------------------------------------------------------
uint32_t ResourceManager::seed() const
{
	return m_seed;
}

//-----------------------------------------------------------------------------
void ResourceManager::poll_resource_loader()
{
	if (m_loader.num_loaded() != 0)
	{
		TempAllocator1024 alloc;
		List<LoadedResource> loaded(alloc);
		m_loader.get_loaded(loaded);

		for (uint32_t i = 0; i < loaded.size(); i++)
		{
			online(loaded[i].resource, loaded[i].data);
		}
	}
}

//-----------------------------------------------------------------------------
ResourceId ResourceManager::load(uint32_t name, uint32_t type)
{
	ResourceId id;
	id.name = name;
	id.type = type;

	// Search for an already existent resource
	ResourceEntry* entry = std::find(m_resources.begin(), m_resources.end(), id);

	// If resource not found, create a new one
	if (entry == m_resources.end())
	{
		id.index = m_resources.size();

		ResourceEntry entry;

		entry.id = id;
		entry.state = RS_UNLOADED;
		entry.references = 1;
		entry.resource = NULL;

		m_resources.push_back(entry);

		// Issue request to resource loader
		m_loader.load(id);

		return id;
	}

	// Else, increment its reference count
	entry->references++;
	
	return entry->id;
}

//-----------------------------------------------------------------------------
void ResourceManager::online(ResourceId name, void* resource)
{
	resource_on_online(name.type, resource);

	ResourceEntry& entry = m_resources[name.index];
	entry.resource = resource;
	entry.state = RS_LOADED;
}

} // namespace crown
