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

#include "Types.h"
#include "ResourceManager.h"
#include "ResourceRegistry.h"
#include "StringUtils.h"
#include "Hash.h"
#include "TempAllocator.h"

namespace crown
{

//-----------------------------------------------------------------------------
ResourceManager::ResourceManager(Bundle& bundle, uint32_t seed) :
	m_resource_heap("resource", default_allocator()),
	m_loader(bundle, m_resource_heap),
	m_seed(seed),
	m_pendings(default_allocator()),
	m_resources(default_allocator())
{
	m_loader.start();
}

//-----------------------------------------------------------------------------
ResourceManager::~ResourceManager()
{
	m_loader.stop();
}

//-----------------------------------------------------------------------------
ResourceId ResourceManager::load(const char* type, const char* name)
{
	return load(resource_id(type, name));
}

//-----------------------------------------------------------------------------
void ResourceManager::unload(ResourceId name)
{
	CE_ASSERT(has(name), "Resource not loaded: %.8X%.8X", name.name, name.type);

	ResourceEntry* entry = find(name);

	entry->references--;
	
	if (entry->references == 0)
	{
		resource_on_unload(name.type, m_resource_heap, entry->resource);
		entry->resource = NULL;
	}
}

//-----------------------------------------------------------------------------
bool ResourceManager::has(ResourceId name) const
{
	ResourceEntry* entry = find(name);

	return entry != NULL;
}

//-----------------------------------------------------------------------------
const void* ResourceManager::data(ResourceId name) const
{
	CE_ASSERT(has(name), "Resource not loaded: %.8X%.8X", name.name, name.type);

	return find(name)->resource;
}

//-----------------------------------------------------------------------------
bool ResourceManager::is_loaded(ResourceId name) const
{
	CE_ASSERT(has(name), "Resource not loaded: %.8X%.8X", name.name, name.type);

	return find(name)->resource != NULL;
}

//-----------------------------------------------------------------------------
uint32_t ResourceManager::references(ResourceId name) const
{
	CE_ASSERT(has(name), "Resource not loaded: %.8X%.8X", name.name, name.type);

	return find(name)->references;
}

//-----------------------------------------------------------------------------
void ResourceManager::flush()
{
	while (!m_pendings.empty())
	{
		poll_resource_loader();
	}
}

//-----------------------------------------------------------------------------
uint32_t ResourceManager::seed() const
{
	return m_seed;
}

//-----------------------------------------------------------------------------
ResourceId ResourceManager::resource_id(const char* type, const char* name) const
{
	ResourceId id;
	id.type = hash::murmur2_32(type, string::strlen(type), 0);
	id.name = hash::murmur2_32(name, string::strlen(name), m_seed);

	return id;
}

//-----------------------------------------------------------------------------
ResourceEntry* ResourceManager::find(ResourceId id) const
{
	const ResourceEntry* entry = std::find(m_resources.begin(), m_resources.end(), id);

	return entry != m_resources.end() ? const_cast<ResourceEntry*>(entry) : NULL;
}

//-----------------------------------------------------------------------------
void ResourceManager::poll_resource_loader()
{
	if (!m_pendings.empty())
	{
		PendingRequest request = m_pendings.front();

		if (m_loader.load_resource_status(request.id) == LRS_LOADED)
		{
			m_pendings.pop_front();

			void* data = m_loader.load_resource_data(request.id);
			online(request.resource, data);
		}
	}
}

//-----------------------------------------------------------------------------
ResourceId ResourceManager::load(ResourceId name)
{
	// Search for an already existent resource
	ResourceEntry* entry = find(name);

	// If resource not found, create a new one
	if (entry == NULL)
	{
		ResourceEntry entry;

		entry.id = name;
		entry.references = 1;
		entry.resource = NULL;

		m_resources.push_back(entry);

		// Issue request to resource loader
		PendingRequest pr;
		pr.resource = name;
		pr.id = m_loader.load_resource(name);

		m_pendings.push_back(pr);

		return name;
	}

	// Else, increment its reference count
	entry->references++;
	
	return entry->id;
}

//-----------------------------------------------------------------------------
void ResourceManager::online(ResourceId name, void* resource)
{
	resource_on_online(name.type, resource);

	ResourceEntry* entry = find(name);
	entry->resource = resource;
}

} // namespace crown
