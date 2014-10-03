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

#include "types.h"
#include "resource.h"
#include "bundle.h"
#include "allocator.h"
#include "file.h"

namespace crown
{

// All offsets are absolute
struct PackageHeader
{
	uint32_t num_textures;
	uint32_t textures_offset;
	uint32_t num_scripts;
	uint32_t scripts_offset;
	uint32_t num_sounds;
	uint32_t sounds_offset;
	uint32_t num_meshes;
	uint32_t meshes_offset;
	uint32_t num_units;
	uint32_t units_offset;
	uint32_t num_sprites;
	uint32_t sprites_offset;
	uint32_t num_physics;
	uint32_t physics_offset;
	uint32_t num_materials;
	uint32_t materials_offset;
	uint32_t num_fonts;
	uint32_t fonts_offset;
	uint32_t num_levels;
	uint32_t levels_offset;
	uint32_t num_physics_configs;
	uint32_t physics_configs_offset;
	uint32_t num_shaders;
	uint32_t shaders_offset;
	uint32_t num_sprite_animations;
	uint32_t sprite_animations_offset;
};

struct PackageResource
{
	//-----------------------------------------------------------------------------
	uint32_t num_textures() const
	{
		return ((PackageHeader*) this)->num_textures;
	}

	//-----------------------------------------------------------------------------
	uint32_t num_scripts() const
	{
		return ((PackageHeader*) this)->num_scripts;
	}

	//-----------------------------------------------------------------------------
	uint32_t num_sounds() const
	{
		return ((PackageHeader*) this)->num_sounds;
	}

	//-----------------------------------------------------------------------------
	uint32_t num_meshes() const
	{
		return ((PackageHeader*) this)->num_meshes;
	}

	//-----------------------------------------------------------------------------
	uint32_t num_units() const
	{
		return ((PackageHeader*) this)->num_units;
	}

	//-----------------------------------------------------------------------------
	uint32_t num_sprites() const
	{
		return ((PackageHeader*) this)->num_sprites;
	}

	//-----------------------------------------------------------------------------
	uint32_t num_physics() const
	{
		return ((PackageHeader*) this)->num_physics;
	}

	//-----------------------------------------------------------------------------
	uint32_t num_materials() const
	{
		return ((PackageHeader*) this)->num_materials;
	}

	//-----------------------------------------------------------------------------
	uint32_t num_fonts() const
	{
		return ((PackageHeader*) this)->num_fonts;
	}

	//-----------------------------------------------------------------------------
	uint32_t num_levels() const
	{
		return ((PackageHeader*) this)->num_levels;
	}

	//-----------------------------------------------------------------------------
	uint32_t num_physics_configs() const
	{
		return ((PackageHeader*) this)->num_physics_configs;
	}

	//-----------------------------------------------------------------------------
	uint32_t num_shaders() const
	{
		return ((PackageHeader*) this)->num_shaders;
	}

	//-----------------------------------------------------------------------------
	uint32_t num_sprite_animations() const
	{
		return ((PackageHeader*) this)->num_sprite_animations;
	}

	//-----------------------------------------------------------------------------
	ResourceId get_texture_id(uint32_t i) const
	{
		CE_ASSERT(i < num_textures(), "Index out of bounds");

		ResourceId* begin = (ResourceId*) ((char*) this + ((PackageHeader*) this)->textures_offset);
		return begin[i];
	}

	//-----------------------------------------------------------------------------
	ResourceId get_script_id(uint32_t i) const
	{
		CE_ASSERT(i < num_scripts(), "Index out of bounds");

		ResourceId* begin = (ResourceId*) ((char*) this + ((PackageHeader*) this)->scripts_offset);
		return begin[i];
	}

	//-----------------------------------------------------------------------------
	ResourceId get_sound_id(uint32_t i) const
	{
		CE_ASSERT(i < num_sounds(), "Index out of bounds");

		ResourceId* begin = (ResourceId*) ((char*) this + ((PackageHeader*) this)->sounds_offset);
		return begin[i];
	}

	//-----------------------------------------------------------------------------
	ResourceId get_mesh_id(uint32_t i) const
	{
		CE_ASSERT(i < num_meshes(), "Index out of bounds");

		ResourceId* begin = (ResourceId*) ((char*) this + ((PackageHeader*) this)->meshes_offset);
		return begin[i];
	}

	//-----------------------------------------------------------------------------
	ResourceId get_unit_id(uint32_t i) const
	{
		CE_ASSERT(i < num_units(), "Index out of bounds");

		ResourceId* begin = (ResourceId*) ((char*) this + ((PackageHeader*) this)->units_offset);
		return begin[i];
	}

	//-----------------------------------------------------------------------------
	ResourceId get_sprite_id(uint32_t i) const
	{
		CE_ASSERT(i < num_sprites(), "Index out of bounds");

		ResourceId* begin = (ResourceId*) ((char*) this + ((PackageHeader*) this)->sprites_offset);
		return begin[i];
	}

	//-----------------------------------------------------------------------------
	ResourceId get_physics_id(uint32_t i) const
	{
		CE_ASSERT(i < num_physics(), "Index out of bounds");

		ResourceId* begin = (ResourceId*) ((char*) this + ((PackageHeader*) this)->physics_offset);
		return begin[i];
	}

	//-----------------------------------------------------------------------------
	ResourceId get_material_id(uint32_t i) const
	{
		CE_ASSERT(i < num_materials(), "Index out of bounds");

		ResourceId* begin = (ResourceId*) ((char*) this + ((PackageHeader*) this)->materials_offset);
		return begin[i];
	}

	//-----------------------------------------------------------------------------
	ResourceId get_font_id(uint32_t i) const
	{
		CE_ASSERT(i < num_fonts(), "Index out of bounds");

		ResourceId* begin = (ResourceId*) ((char*) this + ((PackageHeader*) this)->fonts_offset);
		return begin[i];
	}

	//-----------------------------------------------------------------------------
	ResourceId get_level_id(uint32_t i) const
	{
		CE_ASSERT(i < num_levels(), "Index out of bounds");

		ResourceId* begin = (ResourceId*) ((char*) this + ((PackageHeader*) this)->levels_offset);
		return begin[i];
	}

	//-----------------------------------------------------------------------------
	ResourceId get_physics_config_id(uint32_t i) const
	{
		CE_ASSERT(i < num_physics_configs(), "Index out of bounds");

		ResourceId* begin = (ResourceId*) ((char*) this + ((PackageHeader*) this)->physics_configs_offset);
		return begin[i];
	}

	//-----------------------------------------------------------------------------
	ResourceId get_shader_id(uint32_t i) const
	{
		CE_ASSERT(i < num_shaders(), "Index out of bounds");

		ResourceId* begin = (ResourceId*) ((char*) this + ((PackageHeader*) this)->shaders_offset);
		return begin[i];
	}

	//-----------------------------------------------------------------------------
	ResourceId get_sprite_animation_id(uint32_t i) const
	{
		CE_ASSERT(i < num_sprite_animations(), "Index out of bounds");

		ResourceId* begin = (ResourceId*) ((char*) this + ((PackageHeader*) this)->sprite_animations_offset);
		return begin[i];
	}

private:

	// Disable construction
	PackageResource();
};

namespace package_resource
{
	void compile(Filesystem& fs, const char* resource_path, File* out_file);
	inline void compile(const char* path, CompileOptions& opts)
	{
		compile(opts._fs, path, &opts._bw.m_file);
	}
	void* load(Allocator& allocator, Bundle& bundle, ResourceId id);
	void online(StringId64 /*id*/, ResourceManager& /*rm*/);
	void offline(StringId64 /*id*/, ResourceManager& /*rm*/);
	void unload(Allocator& allocator, void* resource);
} // namespace package_resource
} // namespace crown
