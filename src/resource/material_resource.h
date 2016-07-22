/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "compiler_types.h"
#include "filesystem_types.h"
#include "math_types.h"
#include "memory_types.h"
#include "resource_types.h"
#include "string_id.h"

namespace crown
{
struct MaterialResource
{
	u32 version;
	StringId32 shader;
	u32 num_textures;
	u32 texture_data_offset;
	u32 num_uniforms;
	u32 uniform_data_offset;
	u32 dynamic_data_size;
	u32 dynamic_data_offset;
};

struct TextureData
{
	u32 sampler_name_offset; // Sampler uniform name
	u32 _pad;
	StringId64 id;           // Resource name
	u32 data_offset;         // Offset into dynamic blob
	u32 _pad1;
};

struct TextureHandle
{
	u32 sampler_handle;
	u32 texture_handle;
};

struct UniformType
{
	enum Enum
	{
		FLOAT,
		VECTOR2,
		VECTOR3,
		VECTOR4,
		COUNT
	};
};

struct UniformData
{
	u32 type;        // UniformType::Enum
	StringId32 name; // Uniform name
	u32 name_offset; // Uniform name (string)
	u32 data_offset; // Offset into dynamic blob
};

struct UniformHandle
{
	u32 uniform_handle;
	// data
};

namespace material_resource_internal
{
	void compile(const char* path, CompileOptions& opts);
	void* load(File& file, Allocator& a);
	void online(StringId64 id, ResourceManager& rm);
	void offline(StringId64 id, ResourceManager& rm);
	void unload(Allocator& a, void* res);
} // namespace material_resource_internal

namespace material_resource
{
	UniformData* get_uniform_data(const MaterialResource* mr, u32 i);
	UniformData* get_uniform_data_by_name(const MaterialResource* mr, StringId32 name);
	const char* get_uniform_name(const MaterialResource* mr, const UniformData* ud);
	TextureData* get_texture_data(const MaterialResource* mr, u32 i);
	const char* get_texture_name(const MaterialResource* mr, const TextureData* td);
	UniformHandle* get_uniform_handle(const MaterialResource* mr, u32 i, char* dynamic);
	UniformHandle* get_uniform_handle_by_name(const MaterialResource* mr, StringId32 name, char* dynamic);
	TextureHandle* get_texture_handle(const MaterialResource* mr, u32 i, char* dynamic);
} // namespace material_resource

} // namespace crown
