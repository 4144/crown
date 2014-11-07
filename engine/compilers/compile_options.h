/*
 * Copyright (c) 2012-2014 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "filesystem.h"
#include "reader_writer.h"
#include "crown.h"

namespace crown
{

typedef Array<char> Buffer;

struct CompileOptions
{
	CompileOptions(Filesystem& fs, File* out, Platform::Enum platform)
		: _fs(fs)
		, _bw(*out)
		, _platform(platform)
	{
	}

	Buffer read(const char* path)
	{
		File* file = _fs.open(path, FOM_READ);
		size_t size = file->size();
		Buffer buf(default_allocator());
		array::resize(buf, size);
		file->read(array::begin(buf), size);
		_fs.close(file);
		return buf;
	}

	void get_absolute_path(const char* path, DynamicString& abs)
	{
		_fs.get_absolute_path(path, abs);
	}

	void delete_file(const char* path)
	{
		_fs.delete_file(path);
	}

	BinaryWriter& write(const void* data, size_t size)
	{
		_bw.write(data, size);
		return _bw;
	}

	template <typename T>
	BinaryWriter& write(const T& data)
	{
		_bw.write(data);
		return _bw;
	}

	BinaryWriter& write(const Buffer& data)
	{
		if (array::size(data))
			_bw.write(array::begin(data), array::size(data));

		return _bw;
	}

	Platform::Enum platform() const
	{
		return _platform;
	}

	Filesystem& _fs;
	BinaryWriter _bw;
	Platform::Enum _platform;
};

} // namespace crown
