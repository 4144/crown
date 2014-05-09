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

#pragma once

#include "Allocator.h"
#include "Assert.h"
#include "Bundle.h"
#include "File.h"
#include "Resource.h"
#include "Types.h"
#include "Vector3.h"
#include "Quaternion.h"

namespace crown
{

struct LevelHeader
{
	uint32_t num_units;
	uint32_t units_offset;
	uint32_t num_sounds;
	uint32_t sounds_offset;
};

struct LevelUnit
{
	ResourceId name;
	Vector3 position;
	Quaternion rotation;
};

struct LevelSound
{
	ResourceId name;
	Vector3 position;
	float volume;
	float range;
	bool loop;
};

struct LevelResource
{
	//-----------------------------------------------------------------------------
	static void* load(Allocator& allocator, Bundle& bundle, ResourceId id)
	{
		File* file = bundle.open(id);
		const size_t file_size = file->size();

		void* res = allocator.allocate(file_size);
		file->read(res, file_size);

		bundle.close(file);

		return res;
	}

	//-----------------------------------------------------------------------------
	static void online(void* /*resource*/)
	{
	}

	//-----------------------------------------------------------------------------
	static void unload(Allocator& allocator, void* resource)
	{
		CE_ASSERT_NOT_NULL(resource);
		allocator.deallocate(resource);
	}

	//-----------------------------------------------------------------------------
	static void offline(void* /*resource*/)
	{
	}

	//-----------------------------------------------------------------------------
	uint32_t num_units() const
	{
		return ((LevelHeader*) this)->num_units;
	}

	//-----------------------------------------------------------------------------
	const LevelUnit* get_unit(uint32_t i) const
	{
		CE_ASSERT(i < num_units(), "Index out of bounds");

		const LevelHeader* h = (LevelHeader*) this;
		const LevelUnit* begin = (LevelUnit*) (((char*) this) + h->units_offset);
		return &begin[i];
	}

	//-----------------------------------------------------------------------------
	uint32_t num_sounds() const
	{
		return ((LevelHeader*) this)->num_sounds;
	}

	//-----------------------------------------------------------------------------
	const LevelSound* get_sound(uint32_t i) const
	{
		CE_ASSERT(i < num_sounds(), "Index out of bounds");

		const LevelHeader* h = (LevelHeader*) this;
		const LevelSound* begin = (LevelSound*) (((char*) this) + h->sounds_offset);
		return &begin[i];
	}
};

} // namespace crown
