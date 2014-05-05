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
#include <inttypes.h>
#include "Types.h"
#include "ResourceManager.h"
#include "ResourceRegistry.h"
#include "StringUtils.h"
#include "StringUtils.h"
#include "TempAllocator.h"
#include "DynamicString.h"
#include "Queue.h"

namespace crown
{

ResourceId::ResourceId(const char* type, const char* name)
{
	TempAllocator256 alloc;
	DynamicString res_name(alloc);
	res_name += name;
	res_name += '.';
	res_name += type;

	id = string::murmur2_64(res_name.c_str(), string::strlen(res_name.c_str()), 0);
}

ResourceId::ResourceId(const char* name)
{
	id = string::murmur2_64(name, string::strlen(name), 0);
}

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
	return load(string::murmur2_32(type, string::strlen(type), 0), ResourceId(type, name));
}

//-----------------------------------------------------------------------------
void ResourceManager::unload(ResourceId name, bool force)
{
	CE_ASSERT(has(name), "Resource not loaded: " "%.16"PRIx64"", name.id);

	ResourceEntry* entry = find(name);

	entry->references--;
	
	if (entry->references == 0 || force)
	{
		resource_on_offline(entry->type, entry->resource);
		resource_on_unload(entry->type, m_resource_heap, entry->resource);

		hash::remove(m_resources, name.id);
	}
}

//-----------------------------------------------------------------------------
bool ResourceManager::has(ResourceId name) const
{
	ResourceEntry* entry = find(name);

	return entry != NULL;
}

//-----------------------------------------------------------------------------
const void* ResourceManager::get(const char* type, const char* name) const
{

	ResourceEntry* entry = find(ResourceId(type, name));

	CE_ASSERT(entry != NULL, "Resource not loaded: type = '%s', name = '%s'", type, name);
	CE_ASSERT(entry->resource != NULL, "Resource not loaded: type = '%s', name = '%s'", type, name);

	return entry->resource;
}

//-----------------------------------------------------------------------------
const void* ResourceManager::get(ResourceId name) const
{
	CE_ASSERT(has(name), "Resource not loaded: " "%.16"PRIx64"", name.id);

	return find(name)->resource;
}

//-----------------------------------------------------------------------------
bool ResourceManager::is_loaded(ResourceId name) const
{
	CE_ASSERT(has(name), "Resource not loaded: " "%.16"PRIx64"", name.id);

	return find(name)->resource != NULL;
}

//-----------------------------------------------------------------------------
uint32_t ResourceManager::references(ResourceId name) const
{
	CE_ASSERT(has(name), "Resource not loaded: " "%.16"PRIx64"", name.id);

	return find(name)->references;
}

//-----------------------------------------------------------------------------
void ResourceManager::flush()
{
	while (!queue::empty(m_pendings))
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
ResourceEntry* ResourceManager::find(ResourceId id) const
{
	ResourceEntry deff;
	deff.type = 0xFFFFFFFFu;
	deff.references = 0xFFFFFFFFu;
	deff.resource = NULL;

	const ResourceEntry& entry = hash::get(m_resources, id.id, deff);

	return memcmp(&deff, &entry, sizeof(ResourceEntry)) == 0 ? NULL : const_cast<ResourceEntry*>(&entry);
}

//-----------------------------------------------------------------------------
void ResourceManager::poll_resource_loader()
{
	if (!queue::empty(m_pendings))
	{
		PendingRequest request = queue::front(m_pendings);

		if (m_loader.load_resource_status(request.id) == LRS_LOADED)
		{
			queue::pop_front(m_pendings);

			void* data = m_loader.load_resource_data(request.id);
			online(request.resource, data);
		}
	}
}

//-----------------------------------------------------------------------------
ResourceId ResourceManager::load(uint32_t type, ResourceId name)
{
	// Search for an already existent resource
	ResourceEntry* entry = find(name);

	// If resource not found, create a new one
	if (entry == NULL)
	{
		ResourceEntry new_entry;

		new_entry.type = type;
		new_entry.references = 1;
		new_entry.resource = NULL;

		hash::set(m_resources, name.id, new_entry);

		// Issue request to resource loader
		PendingRequest pr;
		pr.resource = name;
		pr.id = m_loader.load_resource(type, name);

		queue::push_back(m_pendings, pr);

		return name;
	}

	// Else, increment its reference count
	entry->references++;

	return name;
}

//-----------------------------------------------------------------------------
void ResourceManager::online(ResourceId name, void* resource)
{
	ResourceEntry* entry = find(name);
	resource_on_online(entry->type, resource);

	entry->resource = resource;
}

} // namespace crown
