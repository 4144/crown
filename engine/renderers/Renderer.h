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

#include "Config.h"
#include "Types.h"
#include "PixelFormat.h"
#include "VertexFormat.h"
#include "StringUtils.h"
#include "RenderContext.h"
#include "OsThread.h"
#include "OS.h"

namespace crown
{

extern ShaderUniform name_to_stock_uniform(const char* uniform);
class RendererImplementation;

class Renderer
{
public:

	Renderer(Allocator& a);
	~Renderer();

	void init_impl();
	void shutdown_impl();
	void render_impl();

	void create_vertex_buffer_impl(VertexBufferId id, size_t count, VertexFormat format, const void* vertices);
	void create_dynamic_vertex_buffer_impl(VertexBufferId id, size_t count, VertexFormat format);
	void update_vertex_buffer_impl(VertexBufferId id, size_t offset, size_t count, const void* vertices);
	void destroy_vertex_buffer_impl(VertexBufferId id);

	void create_index_buffer_impl(IndexBufferId id, size_t count, const void* indices);
	void create_dynamic_index_buffer_impl(IndexBufferId id, size_t count);
	void update_index_buffer_impl(IndexBufferId id, size_t offset, size_t count, const void* indices);
	void destroy_index_buffer_impl(IndexBufferId id);

	void create_texture_impl(TextureId id, uint32_t width, uint32_t height, PixelFormat format, const void* data);
	void update_texture_impl(TextureId id, uint32_t x, uint32_t y, uint32_t width, uint32_t height, const void* data);
	void destroy_texture_impl(TextureId id);

	void create_shader_impl(ShaderId id, ShaderType type, const char* text);
	void destroy_shader_impl(ShaderId id);

	void create_gpu_program_impl(GPUProgramId id, ShaderId vertex, ShaderId pixel);
	void destroy_gpu_program_impl(GPUProgramId id);

	void create_uniform_impl(UniformId id, const char* name, UniformType type, uint8_t num);
	void update_uniform_impl(UniformId id, size_t size, const void* data);
	void destroy_uniform_impl(UniformId id);

	void create_render_target_impl(RenderTargetId id, uint16_t width, uint16_t height, RenderTargetFormat format);
	void destroy_render_target_impl(RenderTargetId id);

	inline void init()
	{
		m_should_run = true;
		m_thread.start(render_thread, this);

		m_submit->m_commands.write(COMMAND_INIT_RENDERER);
		frame();
	}

	inline void shutdown()
	{
		if (m_should_run)
		{
			m_submit->m_commands.write(COMMAND_SHUTDOWN_RENDERER);
			frame();

			m_thread.stop();		
		}
	}

	/// Creates a new vertex buffer optimized for rendering static vertex data.
	/// @a vertices is the array containig @a count vertex data elements of the given @a format.
	inline VertexBufferId create_vertex_buffer(size_t count, VertexFormat format, const void* vertices)
	{
		const VertexBufferId id = m_vertex_buffers.create();

		m_submit->m_commands.write(COMMAND_CREATE_VERTEX_BUFFER);
		m_submit->m_commands.write(id);
		m_submit->m_commands.write(count);
		m_submit->m_commands.write(format);
		m_submit->m_commands.write(vertices);

		return id;
	}

	inline VertexBufferId create_dynamic_vertex_buffer(size_t count, VertexFormat format)
	{
		const VertexBufferId id = m_vertex_buffers.create();

		m_submit->m_commands.write(COMMAND_CREATE_DYNAMIC_VERTEX_BUFFER);
		m_submit->m_commands.write(id);
		m_submit->m_commands.write(count);
		m_submit->m_commands.write(format);

		return id;
	}

	/// Updates the data associated with the given vertex buffer @a id.
	/// @a vertices is the array containig @a count vertex data elements of the format
	/// specified at the creation of the buffer.
	/// @note
	/// @a count and @a offset together do not have to exceed the number of elements specified
	/// at the creation of the buffer.
	inline void update_vertex_buffer(VertexBufferId id, size_t offset, size_t count, const void* vertices)
	{
		m_submit->m_commands.write(COMMAND_UPDATE_VERTEX_BUFFER);
		m_submit->m_commands.write(id);
		m_submit->m_commands.write(offset);
		m_submit->m_commands.write(count);
		m_submit->m_commands.write(vertices);			
	}

	/// Destroys the given vertex buffer @a id.
	inline void destroy_vertex_buffer(VertexBufferId id)
	{
		m_submit->m_commands.write(COMMAND_DESTROY_VERTEX_BUFFER);
		m_submit->m_commands.write(id);
	}

	/// Creates a new index buffer optimized for rendering static index buffers.
	/// @a indices is the array containing @a count index data elements.
	inline IndexBufferId create_index_buffer(size_t count, const void* indices)
	{
		const IndexBufferId id = m_index_buffers.create();

		m_submit->m_commands.write(COMMAND_CREATE_INDEX_BUFFER);
		m_submit->m_commands.write(id);
		m_submit->m_commands.write(count);
		m_submit->m_commands.write(indices);

		return id;
	}

	inline IndexBufferId create_dynamic_index_buffer(size_t count)
	{
		const IndexBufferId id = m_index_buffers.create();

		m_submit->m_commands.write(COMMAND_CREATE_DYNAMIC_INDEX_BUFFER);
		m_submit->m_commands.write(id);
		m_submit->m_commands.write(count);

		return id;
	}

	inline void update_index_buffer(IndexBufferId id, size_t offset, size_t count, const void* indices)
	{
		m_submit->m_commands.write(COMMAND_UPDATE_INDEX_BUFFER);
		m_submit->m_commands.write(id);
		m_submit->m_commands.write(offset);
		m_submit->m_commands.write(count);
		m_submit->m_commands.write(indices);
	}

	/// Destroys the @a id index buffer.
	inline void destroy_index_buffer(IndexBufferId id)
	{
		m_submit->m_commands.write(COMMAND_DESTROY_INDEX_BUFFER);
		m_submit->m_commands.write(id);
	}

	inline TextureId create_texture(uint32_t width, uint32_t height, PixelFormat format, const void* data)
	{
		const TextureId id = m_textures.create();

		m_submit->m_commands.write(COMMAND_CREATE_TEXTURE);
		m_submit->m_commands.write(id);
		m_submit->m_commands.write(width);
		m_submit->m_commands.write(height);
		m_submit->m_commands.write(format);
		m_submit->m_commands.write(data);

		return id;
	}

	inline void update_texture(TextureId id, uint32_t x, uint32_t y, uint32_t width, uint32_t height, const void* data)
	{
		m_submit->m_commands.write(COMMAND_UPDATE_TEXTURE);
		m_submit->m_commands.write(id);
		m_submit->m_commands.write(x);
		m_submit->m_commands.write(y);
		m_submit->m_commands.write(width);
		m_submit->m_commands.write(height);
		m_submit->m_commands.write(data);
	}

	inline void destroy_texture(TextureId id)
	{
		m_submit->m_commands.write(COMMAND_DESTROY_TEXTURE);
		m_submit->m_commands.write(id);
	}

	inline ShaderId create_shader(ShaderType type, const char* text)
	{
		const ShaderId id = m_shaders.create();

		m_submit->m_commands.write(COMMAND_CREATE_SHADER);
		m_submit->m_commands.write(id);
		m_submit->m_commands.write(type);
		m_submit->m_commands.write(text);

		return id;
	}

	inline void destroy_shader(ShaderId id)
	{
		m_submit->m_commands.write(COMMAND_DESTROY_SHADER);
		m_submit->m_commands.write(id);
	}

	inline GPUProgramId create_gpu_program(ShaderId vertex, ShaderId pixel)
	{
		const GPUProgramId id = m_gpu_programs.create();

		m_submit->m_commands.write(COMMAND_CREATE_GPU_PROGRAM);
		m_submit->m_commands.write(id);
		m_submit->m_commands.write(vertex);
		m_submit->m_commands.write(pixel);

		return id;
	}

	inline void destroy_gpu_program(GPUProgramId id)
	{
		m_submit->m_commands.write(COMMAND_DESTROY_GPU_PROGRAM);
		m_submit->m_commands.write(id);
	}

	inline UniformId create_uniform(const char* name, UniformType type, uint8_t num)
	{
		CE_ASSERT(name_to_stock_uniform(name) == UNIFORM_COUNT, "Uniform name '%s' is a stock uniform.", name);

		const UniformId id = m_uniforms.create();
		size_t len = string::strlen(name);

		CE_ASSERT(len < CROWN_MAX_UNIFORM_NAME_LENGTH, "Max uniform name length is %d", CROWN_MAX_UNIFORM_NAME_LENGTH);

		m_submit->m_commands.write(COMMAND_CREATE_UNIFORM);
		m_submit->m_commands.write(id);
		m_submit->m_commands.write(len);
		m_submit->m_commands.write(name, len);
		m_submit->m_commands.write(type);
		m_submit->m_commands.write(num);

		return id;
	}

	inline void destroy_uniform(UniformId id)
	{
		m_submit->m_commands.write(COMMAND_DESTROY_UNIFORM);
		m_submit->m_commands.write(id);
	}

	// 
	// RenderTargetId create_render_target(uint16_t width, uint16_t height, RenderTargetFormat format)
	// {

	// }
	// void destroy_render_target(RenderTargetId id)
	// {

	// }

	//-----------------------------------------------------------------------------
	inline void execute_commands(CommandBuffer& cmds)
	{
		bool end = false;

		do
		{
			CommandType command;
			cmds.read(command);

			switch (command)
			{
				case COMMAND_INIT_RENDERER:
				{
					init_impl();
					m_is_initialized = true;
					break;
				}
				case COMMAND_SHUTDOWN_RENDERER:
				{
					shutdown_impl();
					m_is_initialized = false;
					m_should_run = false;
					break;
				}
				case COMMAND_CREATE_VERTEX_BUFFER:
				{
					VertexBufferId id;
					size_t count;
					VertexFormat format;
					void* vertices;

					cmds.read(id);
					cmds.read(count);
					cmds.read(format);
					cmds.read(vertices);

					create_vertex_buffer_impl(id, count, format, vertices);
					break;
				}
				case COMMAND_CREATE_DYNAMIC_VERTEX_BUFFER:
				{
					VertexBufferId id;
					size_t count;
					VertexFormat format;

					cmds.read(id);
					cmds.read(count);
					cmds.read(format);

					create_dynamic_vertex_buffer_impl(id, count, format);
					break;
				}
				case COMMAND_UPDATE_VERTEX_BUFFER:
				{
					VertexBufferId id;
					size_t offset;
					size_t count;
					void* vertices;

					cmds.read(id);
					cmds.read(offset);
					cmds.read(count);
					cmds.read(vertices);

					update_vertex_buffer_impl(id, offset, count, vertices);			
					break;
				}
				case COMMAND_DESTROY_VERTEX_BUFFER:
				{
					VertexBufferId id;
					cmds.read(id);

					destroy_vertex_buffer_impl(id);
					break;
				}
				case COMMAND_CREATE_INDEX_BUFFER:
				{
					IndexBufferId id;
					size_t count;
					void* indices;

					cmds.read(id);
					cmds.read(count);
					cmds.read(indices);

					create_index_buffer_impl(id, count, indices);
					break;
				}
				case COMMAND_CREATE_DYNAMIC_INDEX_BUFFER:
				{
					IndexBufferId id;
					size_t count;

					cmds.read(id);
					cmds.read(count);

					create_dynamic_index_buffer_impl(id, count);
					break;
				}
				case COMMAND_UPDATE_INDEX_BUFFER:
				{
					IndexBufferId id;
					size_t offset;
					size_t count;
					void* indices;

					cmds.read(id);
					cmds.read(offset);
					cmds.read(count);
					cmds.read(indices);

					update_index_buffer_impl(id, offset, count, indices);
					break;
				}
				case COMMAND_DESTROY_INDEX_BUFFER:
				{
					IndexBufferId id;
					cmds.read(id);

					destroy_index_buffer_impl(id);
					break;
				}
				case COMMAND_CREATE_TEXTURE:
				{
					TextureId id;
					uint32_t width;
					uint32_t height;
					PixelFormat format;
					void* data;

					cmds.read(id);
					cmds.read(width);
					cmds.read(height);
					cmds.read(format);
					cmds.read(data);

					create_texture_impl(id, width, height, format, data);
					break;
				}
				case COMMAND_UPDATE_TEXTURE:
				{
					TextureId id;
					uint32_t x;
					uint32_t y;
					uint32_t width;
					uint32_t height;
					void* data;

					cmds.read(id);
					cmds.read(x);
					cmds.read(y);
					cmds.read(width);
					cmds.read(height);
					cmds.read(data);

					update_texture_impl(id, x, y, width, height, data);
					break;
				}
				case COMMAND_DESTROY_TEXTURE:
				{
					TextureId id;
					cmds.read(id);

					destroy_texture_impl(id);
					break;
				}
				case COMMAND_CREATE_SHADER:
				{
					ShaderId id;
					ShaderType type;
					char* text;

					cmds.read(id);
					cmds.read(type);
					cmds.read(text);

					create_shader_impl(id, type, text);
					break;
				}
				case COMMAND_DESTROY_SHADER:
				{
					ShaderId id;
					cmds.read(id);

					destroy_shader_impl(id);
					break;
				}
				case COMMAND_CREATE_GPU_PROGRAM:
				{
					GPUProgramId id;
					ShaderId vertex;
					ShaderId pixel;

					cmds.read(id);
					cmds.read(vertex);
					cmds.read(pixel);

					create_gpu_program_impl(id, vertex, pixel);
					break;
				}
				case COMMAND_DESTROY_GPU_PROGRAM:
				{
					GPUProgramId id;
					cmds.read(id);

					destroy_gpu_program_impl(id);
					break;
				}
				case COMMAND_CREATE_UNIFORM:
				{
					UniformId id;
					size_t len;
					char name[CROWN_MAX_UNIFORM_NAME_LENGTH];
					UniformType type;
					uint8_t num;

					cmds.read(id);
					cmds.read(len);
					cmds.read(name, len);
					name[len] = '\0';
					cmds.read(type);
					cmds.read(num);

					create_uniform_impl(id, name, type, num);

					break;
				}
				case COMMAND_DESTROY_UNIFORM:
				{
					UniformId id;
					cmds.read(id);

					destroy_uniform_impl(id);

					break;
				}
				case COMMAND_END:
				{
					end = true;
					break;
				}
				default:
				{
					CE_ASSERT(false, "Oops, unknown command");
					break;
				}
			}
		}
		while (!end);

		cmds.clear();
	}

	inline void update_uniforms(ConstantBuffer& cbuf)
	{
		UniformType type;

		while ((type = (UniformType)cbuf.read()) != UNIFORM_END)
		{
			UniformId id;
			size_t size;

			cbuf.read(&id, sizeof(UniformId));
			cbuf.read(&size, sizeof(size_t));
			const void* data = cbuf.read(size);

			update_uniform_impl(id, size, data);
		}

		cbuf.clear();
	}

	inline void set_state(uint64_t flags)
	{
		m_submit->set_state(flags);
	}

	inline void set_pose(const Mat4& pose)
	{
		m_submit->set_pose(pose);
	}

	inline void set_program(GPUProgramId program)
	{
		m_submit->set_program(program);
	}

	inline void set_vertex_buffer(VertexBufferId vb)
	{
		m_submit->set_vertex_buffer(vb);
	}

	inline void set_index_buffer(IndexBufferId ib)
	{
		m_submit->set_index_buffer(ib);
	}

	inline void set_uniform(UniformId id, UniformType type, void* value, uint8_t num)
	{
		m_submit->set_uniform(id, type, value, num);
	}

	inline void set_texture(uint8_t unit, UniformId sampler_uniform, TextureId texture, uint32_t flags)
	{
		m_submit->set_texture(unit, sampler_uniform, texture, flags);
	}

	inline void set_layer_render_target(uint8_t layer, RenderTargetId target)
	{
		m_submit->set_layer_render_target(layer, target);
	}

	inline void set_layer_clear(uint8_t layer, uint8_t flags, const Color4& color, float depth)
	{
		m_submit->set_layer_clear(layer, flags, color, depth);
	}

	inline void set_layer_view(uint8_t layer, const Mat4& view)
	{
		m_submit->set_layer_view(layer, view);
	}

	inline void set_layer_projection(uint8_t layer, const Mat4& projection)
	{
		m_submit->set_layer_projection(layer, projection);
	}

	inline void set_layer_viewport(uint8_t layer, uint16_t x, uint16_t y, uint16_t width, uint16_t height)
	{
		m_submit->set_layer_viewport(layer, x, y, width, height);
	}

	inline void set_layer_scissor(uint8_t layer, uint16_t x, uint16_t y, uint16_t width, uint16_t height)
	{
		m_submit->set_layer_scissor(layer, x, y, width, height);
	}

	inline void commit(uint8_t layer)
	{
		m_submit->commit(layer);
	}

	static int32_t render_thread(void* thiz)
	{
		Renderer* renderer = (Renderer*)thiz;
		while (renderer->m_should_run)
		{
			renderer->render_all();
		}

		return 0;
	}

	inline void swap_contexts()
	{
		// Ensure COMMAND_END at the end of submit command buffer
		m_submit->push();

		RenderContext* temp = m_submit;
		m_submit = m_draw;
		m_draw = temp;
	}

	inline void frame()
	{
		// Signal main thread finished updating
		m_render_wait.post();
		m_main_wait.wait();
	}

	// Do all the processing needed to render a frame
	inline void render_all()
	{
		// Waits for main thread to finish update
		m_render_wait.wait();

		swap_contexts();

		execute_commands(m_draw->m_commands);
		update_uniforms(m_draw->m_constants);

		if (m_is_initialized)
		{
			render_impl();
		}

		m_main_wait.post();
	}

protected:

	Allocator& m_allocator;
	RendererImplementation* m_impl;

	OsThread m_thread;
	Semaphore m_render_wait;
	Semaphore m_main_wait;

	RenderContext m_contexts[2];
	RenderContext* m_submit;
	RenderContext* m_draw;

	// Id tables
	IdTable<CROWN_MAX_VERTEX_BUFFERS> m_vertex_buffers;
	IdTable<CROWN_MAX_INDEX_BUFFERS> m_index_buffers;
	IdTable<CROWN_MAX_TEXTURES> m_textures;
	IdTable<CROWN_MAX_SHADERS> m_shaders;
	IdTable<CROWN_MAX_GPU_PROGRAMS> m_gpu_programs;
	IdTable<CROWN_MAX_UNIFORMS> m_uniforms;
	// IdTable<CROWN_MAX_RENDER_TARGETS> m_render_targets;

	bool m_is_initialized;
	bool m_should_run;
};

} // namespace crown
