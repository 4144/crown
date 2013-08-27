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

#include "Types.h"
#include "Resource.h"
#include "Bundle.h"
#include "Allocator.h"

namespace crown
{

const uint32_t SOUND_VERSION = 1;

struct SoundHeader
{
	uint32_t	version;	// Sound file version
	uint32_t	size;
	uint32_t	channels;
	uint32_t 	bits_per_sample;
};

class SoundResource
{
public:

	//-----------------------------------------------------------------------------
	static void* load(Allocator& allocator, Bundle& bundle, ResourceId id)
	{
		File* file = bundle.open(id);

		CE_ASSERT(file != NULL, "Resource does not exist: %.8X%.8X", id.name, id.type);

		SoundResource* resource = (SoundResource*)allocator.allocate(sizeof(SoundResource));

		file->read(&resource->m_header, sizeof(SoundHeader));

		size_t size = resource->size();

		resource->m_data = (uint8_t*)allocator.allocate(sizeof(uint8_t) * size);

		file->read(resource->m_data, size);

		bundle.close(file);

		return resource;
	}

	//-----------------------------------------------------------------------------
	static void online(void* resource)
	{
		CE_ASSERT(resource != NULL, "Resource not loaded");
	}

	//-----------------------------------------------------------------------------
	static void unload(Allocator& allocator, void* resource)
	{
		CE_ASSERT(resource != NULL, "Resource not loaded");

		allocator.deallocate(((SoundResource*)resource)->m_data);
		allocator.deallocate(resource);
	}

	//-----------------------------------------------------------------------------
	static void offline(void* /*resource*/)
	{

	}

public:

	uint32_t 			size() const { return m_header.size; }
	uint32_t			channels() const { return m_header.channels; }
	uint32_t			bits_per_sample() const { return m_header.bits_per_sample; }
	const uint8_t*		data() const { return m_data; }

private:

	SoundHeader		m_header;
	uint8_t*		m_data;

};

} // namespace crown