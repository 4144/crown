/*
 * Copyright (c) 2012-2017 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "container_types.h"
#include "filesystem_types.h"
#include "memory_types.h"
#include "resource_types.h"
#include "string_id.h"
#include <bgfx/bgfx.h>

namespace crown
{
/// Manages shaders.
///
/// @ingroup World
struct ShaderManager
{
	struct ShaderData
	{
		u64 state;
		bgfx::ProgramHandle program;
	};

	typedef HashMap<StringId32, ShaderData> ShaderMap;
	ShaderMap _shader_map;

	void add_shader(StringId32 name, u64 state, bgfx::ProgramHandle program);

public:

	ShaderManager(Allocator& a);

	void* load(File& file, Allocator& a);
	void online(StringId64 id, ResourceManager& rm);
	void offline(StringId64 id, ResourceManager& rm);
	void unload(Allocator& a, void* res);
	void submit(StringId32 shader_id, u8 view_id);
};

} // namespace crown
