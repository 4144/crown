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

#include "material.h"
#include "material_resource.h"
#include "memory.h"
#include "device.h"
#include "resource_manager.h"
#include "texture_resource.h"
#include "material_manager.h"
#include "shader.h"
#include <bgfx.h>

namespace crown
{

using namespace material_resource;

void Material::create(const MaterialResource* mr, MaterialManager& mm)
{
	uint32_t size = dynamic_data_size(mr);
	uint32_t offt = dynamic_data_offset(mr);
	char* base = (char*) mr + offt;
	data = (char*) default_allocator().allocate(size);
	memcpy(data, base, size);
	resource = mr;
}

void Material::destroy() const
{
	default_allocator().deallocate(data);
}

void Material::bind() const
{
	Shader* shader = (Shader*) device()->resource_manager()->get(SHADER_TYPE, material_resource::shader(resource));
	bgfx::setProgram(shader->program);

	// Set samplers
	for (uint32_t i = 0; i < num_textures(resource); i++)
	{
		TextureData* td = get_texture_data(resource, i);
		TextureHandle* th = get_texture_handle(resource, i, data);

		bgfx::UniformHandle sampler;
		bgfx::TextureHandle texture;
		sampler.idx = th->sampler_handle;

		TextureResource* teximg = (TextureResource*) device()->resource_manager()->get(TEXTURE_TYPE, td->id);
		texture.idx = teximg->handle.idx;

		bgfx::setTexture(i, sampler, texture);
	}

	// Set uniforms
	for (uint32_t i = 0; i < num_uniforms(resource); i++)
	{
		UniformHandle* uh = get_uniform_handle(resource, i, data);

		bgfx::UniformHandle buh;
		buh.idx = uh->uniform_handle;
		bgfx::setUniform(buh, (char*) uh + sizeof(uh->uniform_handle));
	}
}

void Material::set_float(const char* name, float val)
{
	char* p = (char*) get_uniform_handle_by_string(resource, name, data);
	*((float*)(p + sizeof(uint32_t))) = val;
}

void Material::set_vector2(const char* name, const Vector2& val)
{
	char* p = (char*) get_uniform_handle_by_string(resource, name, data);
	*((Vector2*)(p + sizeof(uint32_t))) = val;
}

void Material::set_vector3(const char* name, const Vector3& val)
{
	char* p = (char*) get_uniform_handle_by_string(resource, name, data);
	*((Vector3*)(p + sizeof(uint32_t))) = val;
}

} // namespace crown
