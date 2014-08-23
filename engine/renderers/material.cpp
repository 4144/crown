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
#include "random.h"
#include "device.h"
#include "resource_manager.h"
#include "texture_resource.h"
#include "material_manager.h"
#include "log.h"
#include "shader.h"
#include <bgfx.h>

namespace crown
{

void Material::create(const MaterialResource* mr, MaterialManager& mm)
{
	uint32_t size = mr->dynamic_data_size();
	uint32_t offt = mr->dynamic_data_offset();
	char* base = (char*) mr + offt;

	data = (char*) default_allocator().allocate(size);
	memcpy(data, base, size);

	for (uint32_t i = 0; i < mr->num_textures(); i++)
	{
		TextureData* ud = mr->get_texture_data(i);

		TextureHandle* th = mr->get_texture_handle(i, data);
		th->sampler_handle = bgfx::createUniform(ud->sampler_name, bgfx::UniformType::Uniform1iv).idx;

		ResourceId texid;
		texid.type = TEXTURE_TYPE;
		texid.name = ud->id;
		TextureImage* teximg = (TextureImage*) device()->resource_manager()->get(texid);
		th->texture_handle = teximg->handle.idx;
	}

	for (uint32_t i = 0; i < mr->num_uniforms(); i++)
	{
		UniformData* ud = mr->get_uniform_data(i);

		UniformHandle* uh = mr->get_uniform_handle(i, data);
		uh->uniform_handle = bgfx::createUniform(ud->name, bgfx::UniformType::Uniform4fv).idx;
	}

	resource = mr;
}

void Material::clone(const Material& m)
{
	const MaterialResource* mr = m.resource;
	uint32_t size = mr->dynamic_data_size();
	char* base = m.data;

	data = (char*) default_allocator().allocate(size);
	memcpy(data, base, size);

	resource = mr;
}

void Material::destroy() const
{
	for (uint32_t i = 0; i < resource->num_textures(); i++)
	{
		TextureHandle* th = resource->get_texture_handle(i, data);

		bgfx::UniformHandle sh;
		sh.idx = th->sampler_handle;
		bgfx::destroyUniform(sh);
	}

	for (uint32_t i = 0; i < resource->num_uniforms(); i++)
	{
		UniformHandle* uh = resource->get_uniform_handle(i, data);

		bgfx::UniformHandle bgfx_uh;
		bgfx_uh.idx = uh->uniform_handle;
		bgfx::destroyUniform(bgfx_uh);
	}

	default_allocator().deallocate(data);
}

void Material::bind() const
{
	// bgfx::setProgram(program);
	ResourceId shader_id;
	shader_id.type = SHADER_TYPE;
	shader_id.name = resource->shader();
	Shader* shader = (Shader*) device()->resource_manager()->get(shader_id);
	bgfx::setProgram(shader->program);

	// Set samplers
	for (uint32_t i = 0; i < resource->num_textures(); i++)
	{
		TextureHandle* th = resource->get_texture_handle(i, data);

		bgfx::UniformHandle sampler;
		bgfx::TextureHandle texture;
		sampler.idx = th->sampler_handle;
		texture.idx = th->texture_handle;

		bgfx::setTexture(i, sampler, texture);
	}

	// Set uniforms
	for (uint32_t i = 0; i < resource->num_uniforms(); i++)
	{
		UniformHandle* uh = resource->get_uniform_handle(i, data);

		bgfx::UniformHandle buh;
		buh.idx = uh->uniform_handle;
		bgfx::setUniform(buh, (char*) uh + sizeof(uh->uniform_handle));
	}
}

void Material::set_float(const char* name, float val)
{
	char* p = (char*) resource->get_uniform_handle_by_string(name, data);
	*((float*)(p + sizeof(uint32_t))) = val;

}

void Material::set_vector2(const char* name, const Vector2& val)
{
	char* p = (char*) resource->get_uniform_handle_by_string(name, data);
	*((Vector2*)(p + sizeof(uint32_t))) = val;
}

void Material::set_vector3(const char* name, const Vector3& val)
{
	char* p = (char*) resource->get_uniform_handle_by_string(name, data);
	*((Vector3*)(p + sizeof(uint32_t))) = val;
}

} // namespace crown
