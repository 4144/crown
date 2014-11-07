/*
 * Copyright (c) 2012-2014 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "types.h"
#include "thread.h"
#include "container_types.h"
#include "mutex.h"
#include "memory_types.h"
#include "resource_id.h"

namespace crown
{

class Bundle;

struct ResourceData
{
	ResourceId id;
	void* data;
};

/// Loads resources in a background thread.
///
/// @ingroup Resource
class ResourceLoader
{
public:

	/// Reads the resources data from the given @a bundle using
	/// @a resource_heap to allocate memory for them.
	ResourceLoader(Bundle& bundle, Allocator& resource_heap);
	~ResourceLoader();

	/// Loads the @a resource in a background thread.
	void load(ResourceId id);

	/// Blocks until all pending requests have been processed.
	void flush();

	void get_loaded(Array<ResourceData>& loaded);

private:

	void add_request(ResourceId id);
	uint32_t num_requests();
	void add_loaded(ResourceData data);

	// Loads resources in the loading queue.
	int32_t run();

	static int32_t thread_proc(void* thiz)
	{
		ResourceLoader* rl = (ResourceLoader*)thiz;
		return rl->run();
	}

private:

	Thread _thread;
	Bundle& _bundle;
	Allocator& _resource_heap;

	Queue<ResourceId> _requests;
	Queue<ResourceData> _loaded;
	Mutex _mutex;
	Mutex _loaded_mutex;
	bool _exit;
};

} // namespace crown
