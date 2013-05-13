/*
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

#include "Config.h"

#include <GL/glew.h>
#include <cassert>
#include <algorithm>

#include "Types.h"
#include "GLRenderer.h"
#include "GLUtils.h"
#include "Log.h"
#include "Material.h"
#include "Vec2.h"
#include "Vec3.h"
#include "Vec4.h"
#include "Mat3.h"
#include "Mat4.h"
#include "Device.h"
#include "ResourceManager.h"
#include "VertexShaderResource.h"
#include "PixelShaderResource.h"

#if defined(WINDOWS)
	//Define the missing constants in vs' gl.h
	#define GL_TEXTURE_3D					0x806F
	#define GL_TEXTURE_CUBE_MAP				0x8513
	#define GL_LIGHT_MODEL_COLOR_CONTROL	0x81F8
	#define GL_SEPARATE_SPECULAR_COLOR		0x81FA
	#define GL_SINGLE_COLOR					0x81F9
	#define GL_GENERATE_MIPMAP				0x8191
#endif

namespace crown
{

//-----------------------------------------------------------------------------
GLRenderer::GLRenderer() :
	m_max_lights(0),
	m_max_texture_size(0),
	m_max_texture_units(0),
	m_max_vertex_indices(0),
	m_max_vertex_vertices(0),
	m_max_anisotropy(0.0f),

	m_textures_id_table(m_allocator, MAX_TEXTURES),
	m_active_texture_unit(0),

	m_vertex_buffers_id_table(m_allocator, MAX_VERTEX_BUFFERS),
	m_index_buffers_id_table(m_allocator, MAX_INDEX_BUFFERS),
	m_vertex_shaders_id_table(m_allocator, MAX_VERTEX_SHADERS),
	m_pixel_shaders_id_table(m_allocator, MAX_PIXEL_SHADERS),
	m_gpu_programs_id_table(m_allocator, 128)
	//m_render_buffers_id_table(m_allocator, MAX_RENDER_BUFFERS)
{
	m_min_max_point_size[0] = 0.0f;
	m_min_max_point_size[1] = 0.0f;
	m_min_max_line_width[0] = 0.0f;
	m_min_max_line_width[1] = 0.0f;

	// Initialize viewport and scissor
	m_viewport[0] = 0;
	m_viewport[1] = 0;
	m_viewport[2] = 0;
	m_viewport[3] = 0;

	m_scissor[0] = 0;
	m_scissor[1] = 0;
	m_scissor[2] = 0;
	m_scissor[3] = 0;

	// Initialize texture units
	for (uint32_t i = 0; i < MAX_TEXTURE_UNITS; i++)
	{
		m_texture_unit[i] = 0;
		m_texture_unit_target[i] = GL_TEXTURE_2D;
	}

	// Initialize the matrices
	for (uint32_t i = 0; i < MT_COUNT; i++)
	{
		m_matrix[i].load_identity();
	}

	GLenum err = glewInit();

	assert(err == GLEW_OK);

	Log::i("GLEW initialized.");

	glGetIntegerv(GL_MAX_LIGHTS, &m_max_lights);
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &m_max_texture_size);
	glGetIntegerv(GL_MAX_TEXTURE_UNITS, &m_max_texture_units);
	glGetIntegerv(GL_MAX_ELEMENTS_INDICES, &m_max_vertex_indices);
	glGetIntegerv(GL_MAX_ELEMENTS_VERTICES, &m_max_vertex_vertices);

	// Check for anisotropic filter support
	if (GLEW_EXT_texture_filter_anisotropic)
	{
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &m_max_anisotropy);
	}

	glGetFloatv(GL_ALIASED_POINT_SIZE_RANGE, &m_min_max_point_size[0]);
	glGetFloatv(GL_LINE_WIDTH_RANGE, &m_min_max_line_width[0]);

	const unsigned char* gl_vendor = glGetString(GL_VENDOR);
	const unsigned char* gl_renderer = glGetString(GL_RENDERER);
	const unsigned char* gl_version = glGetString(GL_VERSION);

	Log::i("OpenGL Vendor\t: %s", gl_vendor);
	Log::i("OpenGL Renderer\t: %s", gl_renderer);
	Log::i("OpenGL Version\t: %s", gl_version);
	Log::i("Min Point Size\t: %f", m_min_max_point_size[0]);
	Log::i("Max Point Size\t: %f", m_min_max_point_size[1]);
	Log::i("Min Line Width\t: %f", m_min_max_line_width[0]);
	Log::i("Max Line Width\t: %f", m_min_max_line_width[1]);
	Log::i("Max Texture Size\t: %dx%d", m_max_texture_size, m_max_texture_size);
	Log::i("Max Texture Units\t: %d", m_max_texture_units);
	Log::i("Max Lights\t\t: %d", m_max_lights);
	Log::i("Max Vertex Indices\t: %d", m_max_vertex_indices);
	Log::i("Max Vertex Vertices\t: %d", m_max_vertex_vertices);
	Log::i("Max Anisotropy\t: %f", m_max_anisotropy);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glDisable(GL_TEXTURE_2D);

	glEnable(GL_LIGHTING);

	glDisable(GL_BLEND);
	//TODO: Use Premultiplied alpha
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBlendEquation(GL_FUNC_ADD);

	glFrontFace(GL_CCW);

	glEnable(GL_CULL_FACE);

	glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR);

	glShadeModel(GL_SMOOTH);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	// Set the global ambient light
	float amb[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, amb);

	// Some hints
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	// Set the framebuffer clear color
	glClearColor(0.5f, 0.5f, 0.5f, 0.5f);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glClearDepth(1.0);

	// Enable scissor test
	glEnable(GL_SCISSOR_TEST);

	// Disable dithering
	glDisable(GL_DITHER);

	Log::i("OpenGL Renderer initialized.");
}

//-----------------------------------------------------------------------------
GLRenderer::~GLRenderer()
{
}

//-----------------------------------------------------------------------------
void GLRenderer::init()
{
	load_default_shaders();
}

//-----------------------------------------------------------------------------
void GLRenderer::shutdown()
{
	unload_default_shaders();
}

//-----------------------------------------------------------------------------
VertexBufferId GLRenderer::create_vertex_buffer(size_t count, VertexFormat format, const void* vertices)
{
	const VertexBufferId id = m_vertex_buffers_id_table.create();

	GLVertexBuffer& buffer = m_vertex_buffers[id.index];

	glGenBuffers(1, &buffer.gl_object);

	glBindBuffer(GL_ARRAY_BUFFER, buffer.gl_object);
	glBufferData(GL_ARRAY_BUFFER, count * Vertex::bytes_per_vertex(format), vertices, GL_STATIC_DRAW);

	buffer.count = count;
	buffer.format = format;

	return id;
}

//-----------------------------------------------------------------------------
VertexBufferId GLRenderer::create_dynamic_vertex_buffer(size_t count, VertexFormat format, const void* vertices)
{
	const VertexBufferId id = m_vertex_buffers_id_table.create();

	GLVertexBuffer& buffer = m_vertex_buffers[id.index];

	glGenBuffers(1, &buffer.gl_object);

	glBindBuffer(GL_ARRAY_BUFFER, buffer.gl_object);
	glBufferData(GL_ARRAY_BUFFER, count * Vertex::bytes_per_vertex(format), vertices, GL_STREAM_DRAW);

	buffer.count = count;
	buffer.format = format;

	return id;
}

//-----------------------------------------------------------------------------
void GLRenderer::update_vertex_buffer(VertexBufferId id, size_t offset, size_t count, const void* vertices)
{
	assert(m_vertex_buffers_id_table.has(id));

	GLVertexBuffer& buffer = m_vertex_buffers[id.index];

	glBindBuffer(GL_ARRAY_BUFFER, buffer.gl_object);
	glBufferSubData(GL_ARRAY_BUFFER, offset * Vertex::bytes_per_vertex(buffer.format),
					count * Vertex::bytes_per_vertex(buffer.format), vertices);
}

//-----------------------------------------------------------------------------
void GLRenderer::destroy_vertex_buffer(VertexBufferId id)
{
	assert(m_vertex_buffers_id_table.has(id));

	GLVertexBuffer& buffer = m_vertex_buffers[id.index];

	glDeleteBuffers(1, &buffer.gl_object);

	m_vertex_buffers_id_table.destroy(id);
}

//-----------------------------------------------------------------------------
IndexBufferId GLRenderer::create_index_buffer(size_t count, const void* indices)
{
	const IndexBufferId id = m_index_buffers_id_table.create();

	GLIndexBuffer& buffer = m_index_buffers[id.index];

	glGenBuffers(1, &buffer.gl_object);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer.gl_object);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(GLushort), indices, GL_STATIC_DRAW);

	buffer.index_count = count;

	return id;
}

//-----------------------------------------------------------------------------
void GLRenderer::destroy_index_buffer(IndexBufferId id)
{
	assert(m_index_buffers_id_table.has(id));

	GLIndexBuffer& buffer = m_index_buffers[id.index];

	glDeleteBuffers(1, &buffer.gl_object);

	m_index_buffers_id_table.destroy(id);
}

//-----------------------------------------------------------------------------
TextureId GLRenderer::create_texture(uint32_t width, uint32_t height, PixelFormat format, const void* data)
{
	const TextureId id = m_textures_id_table.create();

	GLTexture& gl_texture = m_textures[id.index];

	glGenTextures(1, &gl_texture.gl_object);

	glBindTexture(GL_TEXTURE_2D, gl_texture.gl_object);

	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);

	// FIXME
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
				 GL::pixel_format(format), GL_UNSIGNED_BYTE, data);

	gl_texture.format = format;

	return id;
}

//-----------------------------------------------------------------------------
void GLRenderer::update_texture(TextureId id, uint32_t x, uint32_t y, uint32_t width, uint32_t height, const void* data)
{
	assert(m_textures_id_table.has(id));

	GLTexture& gl_texture = m_textures[id.index];

	glBindTexture(GL_TEXTURE_2D, gl_texture.gl_object);

	glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, width, height, GL::pixel_format(gl_texture.format),
					GL_UNSIGNED_BYTE, data);
}

//-----------------------------------------------------------------------------
void GLRenderer::destroy_texture(TextureId id)
{
	assert(m_textures_id_table.has(id));

	GLTexture& gl_texture = m_textures[id.index];

	glDeleteTextures(1, &gl_texture.gl_object);
}

//-----------------------------------------------------------------------------
VertexShaderId GLRenderer::create_vertex_shader(const char* program)
{
	assert(program != NULL);

	const VertexShaderId& id = m_vertex_shaders_id_table.create();

	GLVertexShader& gl_shader = m_vertex_shaders[id.index];

	gl_shader.gl_object = glCreateShader(GL_VERTEX_SHADER);

	glShaderSource(gl_shader.gl_object, 1, &program, NULL);

	glCompileShader(gl_shader.gl_object);

	GLint success;
	glGetShaderiv(gl_shader.gl_object, GL_COMPILE_STATUS, &success);

	if (!success)
	{
		GLchar info_log[256];

		glGetShaderInfoLog(gl_shader.gl_object, 256, NULL, info_log);

		Log::e("Vertex shader compilation failed.");
		Log::e("Log: %s", info_log);
		assert(0);
	}

	return id;
}

//-----------------------------------------------------------------------------
void GLRenderer::destroy_vertex_shader(VertexShaderId id)
{
	assert(m_vertex_shaders_id_table.has(id));

	GLVertexShader& gl_shader = m_vertex_shaders[id.index];

	glDeleteShader(gl_shader.gl_object);
}

//-----------------------------------------------------------------------------
PixelShaderId GLRenderer::create_pixel_shader(const char* program)
{
	assert(program != NULL);

	const PixelShaderId& id = m_pixel_shaders_id_table.create();

	GLPixelShader& gl_shader = m_pixel_shaders[id.index];

	gl_shader.gl_object = glCreateShader(GL_FRAGMENT_SHADER);

	glShaderSource(gl_shader.gl_object, 1, &program, NULL);

	glCompileShader(gl_shader.gl_object);

	GLint success;
	glGetShaderiv(gl_shader.gl_object, GL_COMPILE_STATUS, &success);

	if (!success)
	{
		GLchar info_log[256];

		glGetShaderInfoLog(gl_shader.gl_object, 256, NULL, info_log);

		Log::e("Pixel shader compilation failed.");
		Log::e("Log: %s", info_log);
		assert(0);
	}

	return id;	
}

//-----------------------------------------------------------------------------
void GLRenderer::destroy_pixel_shader(PixelShaderId id)
{
	assert(m_pixel_shaders_id_table.has(id));

	GLPixelShader& gl_shader = m_pixel_shaders[id.index];

	glDeleteShader(gl_shader.gl_object);	
}

//-----------------------------------------------------------------------------
GPUProgramId GLRenderer::create_gpu_program(VertexShaderId vs, PixelShaderId ps)
{
	assert(m_vertex_shaders_id_table.has(vs));
	assert(m_pixel_shaders_id_table.has(ps));

	const GPUProgramId id = m_gpu_programs_id_table.create();

	GLGPUProgram& gl_program = m_gpu_programs[id.index];

	gl_program.gl_object = glCreateProgram();

	glAttachShader(gl_program.gl_object, m_vertex_shaders[id.index].gl_object);
	glAttachShader(gl_program.gl_object, m_pixel_shaders[id.index].gl_object);

	glBindAttribLocation(gl_program.gl_object, SA_VERTEX, "vertex");
	glBindAttribLocation(gl_program.gl_object, SA_COORDS, "coords");
	glBindAttribLocation(gl_program.gl_object, SA_NORMAL, "normal");

	glLinkProgram(gl_program.gl_object);

	GLint success;
	glGetProgramiv(gl_program.gl_object, GL_LINK_STATUS, &success);

	if (!success)
	{
		GLchar info_log[256];
		glGetProgramInfoLog(gl_program.gl_object, 256, NULL, info_log);
		Log::e("GPU program compilation failed.\n");
		Log::e("Log: %s", info_log);
	}

	return id;
}

//-----------------------------------------------------------------------------
void GLRenderer::destroy_gpu_program(GPUProgramId id)
{
	assert(m_gpu_programs_id_table.has(id));

	GLGPUProgram& gl_program = m_gpu_programs[id.index];

	glDeleteProgram(gl_program.gl_object);
}

//-----------------------------------------------------------------------------
void GLRenderer::set_gpu_program_bool_uniform(GPUProgramId id, const char* name, bool value)
{
	assert(m_gpu_programs_id_table.has(id));

	const GLint uniform = find_gpu_program_uniform(m_gpu_programs[id.index].gl_object, name);

	glUniform1i(uniform, (GLint) value);
}

//-----------------------------------------------------------------------------
void GLRenderer::set_gpu_program_int_uniform(GPUProgramId id, const char* name, int value)
{
	assert(m_gpu_programs_id_table.has(id));

	const GLint uniform = find_gpu_program_uniform(m_gpu_programs[id.index].gl_object, name);

	glUniform1i(uniform, (GLint) value);
}

//-----------------------------------------------------------------------------
void GLRenderer::set_gpu_program_vec2_uniform(GPUProgramId id, const char* name, const Vec2& value)
{
	assert(m_gpu_programs_id_table.has(id));

	const GLint uniform = find_gpu_program_uniform(m_gpu_programs[id.index].gl_object, name);

	glUniform2fv(uniform, 1, value.to_float_ptr());
}

//-----------------------------------------------------------------------------
void GLRenderer::set_gpu_program_vec3_uniform(GPUProgramId id, const char* name, const Vec3& value)
{
	assert(m_gpu_programs_id_table.has(id));

	const GLint uniform = find_gpu_program_uniform(m_gpu_programs[id.index].gl_object, name);

	glUniform3fv(uniform, 1, value.to_float_ptr());
}

//-----------------------------------------------------------------------------
void GLRenderer::set_gpu_program_vec4_uniform(GPUProgramId id, const char* name, const Vec4& value)
{
	assert(m_gpu_programs_id_table.has(id));

	const GLint uniform = find_gpu_program_uniform(m_gpu_programs[id.index].gl_object, name);

	glUniform4fv(uniform, 1, value.to_float_ptr());
}

//-----------------------------------------------------------------------------
void GLRenderer::set_gpu_porgram_mat3_uniform(GPUProgramId id, const char* name, const Mat3& value)
{
	assert(m_gpu_programs_id_table.has(id));

	const GLint uniform = find_gpu_program_uniform(m_gpu_programs[id.index].gl_object, name);

	glUniformMatrix3fv(uniform, 1, GL_TRUE, value.to_float_ptr());
}

//-----------------------------------------------------------------------------
void GLRenderer::set_gpu_program_mat4_uniform(GPUProgramId id, const char* name, const Mat4& value)
{
	assert(m_gpu_programs_id_table.has(id));

	const GLint uniform = find_gpu_program_uniform(m_gpu_programs[id.index].gl_object, name);

	glUniformMatrix4fv(uniform, 1, GL_TRUE, value.to_float_ptr());
}

//-----------------------------------------------------------------------------
void GLRenderer::set_gpu_program_sampler_uniform(GPUProgramId id, const char* name, uint32_t value)
{
	assert(m_gpu_programs_id_table.has(id));

	const GLint uniform = find_gpu_program_uniform(m_gpu_programs[id.index].gl_object, name);

	glUniform1i(uniform, (GLint) value);	
}

//-----------------------------------------------------------------------------
void GLRenderer::bind_gpu_program(GPUProgramId id) const
{
	assert(m_gpu_programs_id_table.has(id));

	const GLGPUProgram& gl_program = m_gpu_programs[id.index];

	glUseProgram(gl_program.gl_object);
}

//-----------------------------------------------------------------------------
// RenderBufferId GLRenderer::create_render_buffer(uint32_t width, uint32_t height, PixelFormat format)
// {
// 	const RenderBufferId id = m_render_buffers_id_table.create();

// 	GLRenderBuffer& buffer = m_render_buffers[id.index];

// 	if (GLEW_EXT_framebuffer_object)
// 	{
// 		glGenFramebuffersEXT(1, &buffer.gl_frame_buffer);
// 		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, buffer.gl_frame_buffer);

// 		glGenRenderbuffersEXT(1, &buffer.gl_render_buffer);
// 		glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, buffer.gl_render_buffer);

// 		glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_RGBA8, width, height);

// 		glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_RENDERBUFFER_EXT, buffer.gl_render_buffer);

// 		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
// 	}

// 	return id;
// }

//-----------------------------------------------------------------------------
// void GLRenderer::destroy_render_buffer(RenderBufferId id)
// {
// 	GLRenderBuffer& buffer = m_render_buffers[id.index];

// 	if (GLEW_EXT_framebuffer_object)
// 	{
// 		glDeleteFramebuffersEXT(1, &buffer.gl_frame_buffer);
// 		glDeleteRenderbuffersEXT(1, &buffer.gl_render_buffer);
// 	}

// 	m_render_buffers_id_table.destroy(id);
// }

//-----------------------------------------------------------------------------
void GLRenderer::set_clear_color(const Color4& color)
{
	glClearColor(color.r, color.g, color.b, color.a);
}

//-----------------------------------------------------------------------------
void GLRenderer::set_material_params(const Color4& ambient, const Color4& diffuse, const Color4& specular,
				const Color4& emission, int32_t shininess)
{
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, &ambient.r);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, &diffuse.r);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, &specular.r);
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, &emission.r);
	glMateriali(GL_FRONT_AND_BACK, GL_SHININESS, shininess);
}

//-----------------------------------------------------------------------------
void GLRenderer::set_ambient_light(const Color4& color)
{
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, color.to_float_ptr());
}

//-----------------------------------------------------------------------------
void GLRenderer::set_lighting(bool lighting)
{
	if (lighting)
	{
		glEnable(GL_LIGHTING);
	}
	else
	{
		glDisable(GL_LIGHTING);
	}
}

//-----------------------------------------------------------------------------
void GLRenderer::bind_texture(uint32_t unit, TextureId texture)
{
	assert(m_textures_id_table.has(texture));

	if (!activate_texture_unit(unit))
	{
		return;
	}

	m_texture_unit_target[unit] = GL_TEXTURE_2D;
	m_texture_unit[unit] = m_textures[texture.index].gl_object;

	glEnable(m_texture_unit_target[unit]);
	glBindTexture(m_texture_unit_target[unit], m_texture_unit[unit]);
}

//-----------------------------------------------------------------------------
void GLRenderer::set_texturing(uint32_t unit, bool texturing)
{
	if (!activate_texture_unit(unit))
		return;

	if (texturing)
	{
		glEnable(m_texture_unit_target[unit]);
	}
	else
	{
		glDisable(m_texture_unit_target[unit]);
	}
}

//-----------------------------------------------------------------------------
void GLRenderer::set_texture_mode(uint32_t unit, TextureMode mode, const Color4& blendColor)
{
	if (!activate_texture_unit(unit))
		return;

	GLint envMode = GL::texture_mode(mode);

	if (envMode == GL_BLEND)
	{
		glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, &blendColor.r);
	}

	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, envMode);
}

//-----------------------------------------------------------------------------
void GLRenderer::set_texture_wrap(uint32_t unit, TextureWrap wrap)
{
	GLenum glWrap = GL::texture_wrap(wrap);

	glTexParameteri(m_texture_unit_target[unit], GL_TEXTURE_WRAP_S, glWrap);
	glTexParameteri(m_texture_unit_target[unit], GL_TEXTURE_WRAP_T, glWrap);
	glTexParameteri(m_texture_unit_target[unit], GL_TEXTURE_WRAP_R, glWrap);
}

//-----------------------------------------------------------------------------
void GLRenderer::set_texture_filter(uint32_t unit, TextureFilter filter)
{
	if (!activate_texture_unit(unit))
		return;

	GLint minFilter;
	GLint magFilter;

	GL::texture_filter(filter, minFilter, magFilter);

	glTexParameteri(m_texture_unit_target[unit], GL_TEXTURE_MIN_FILTER, minFilter);
	glTexParameteri(m_texture_unit_target[unit], GL_TEXTURE_MAG_FILTER, magFilter);
}

//-----------------------------------------------------------------------------
void GLRenderer::set_backface_culling(bool culling)
{
	if (culling)
	{
		glEnable(GL_CULL_FACE);
	}
	else
	{
		glDisable(GL_CULL_FACE);
	}
}

//-----------------------------------------------------------------------------
void GLRenderer::set_separate_specular_color(bool separate)
{
	if (separate)
	{
		glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR);
	}
	else
	{
		glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SINGLE_COLOR);
	}
}

//-----------------------------------------------------------------------------
void GLRenderer::set_depth_test(bool test)
{
	if (test)
	{
		glEnable(GL_DEPTH_TEST);
	}
	else
	{
		glDisable(GL_DEPTH_TEST);
	}
}

//-----------------------------------------------------------------------------
void GLRenderer::set_depth_write(bool write)
{
	glDepthMask((GLboolean) write);
}

//-----------------------------------------------------------------------------
void GLRenderer::set_depth_func(CompareFunction func)
{
	GLenum glFunc = GL::compare_function(func);

	glDepthFunc(glFunc);
}

//-----------------------------------------------------------------------------
void GLRenderer::set_rescale_normals(bool rescale)
{
	if (rescale)
	{
		glEnable(GL_RESCALE_NORMAL);
	}
	else
	{
		glDisable(GL_RESCALE_NORMAL);
	}
}

//-----------------------------------------------------------------------------
void GLRenderer::set_blending(bool blending)
{
	if (blending)
	{
		glEnable(GL_BLEND);
	}
	else
	{
		glDisable(GL_BLEND);
	}
}

//-----------------------------------------------------------------------------
void GLRenderer::set_blending_params(BlendEquation equation, BlendFunction src, BlendFunction dst, const Color4& color)
{
	GLenum glEquation = GL::blend_equation(equation);

	glBlendEquation(glEquation);

	GLenum glSrcFactor = GL::blend_function(src);
	GLenum glDstFactor = GL::blend_function(dst);

	glBlendFunc(glSrcFactor, glDstFactor);

	glBlendColor(color.r, color.g, color.b, color.a);
}

//-----------------------------------------------------------------------------
void GLRenderer::set_color_write(bool write)
{
	if (write)
	{
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	}
	else
	{
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	}
}

//-----------------------------------------------------------------------------
void GLRenderer::set_fog(bool fog)
{
	if (fog)
	{
		glEnable(GL_FOG);
	}
	else
	{
		glDisable(GL_FOG);
	}
}

//-----------------------------------------------------------------------------
void GLRenderer::set_fog_params(FogMode mode, float density, float start, float end, const Color4& color)
{
	GLenum glMode = GL::fog_mode(mode);

	glFogi(GL_FOG_MODE, glMode);
	glFogf(GL_FOG_DENSITY, density);
	glFogf(GL_FOG_START, start);
	glFogf(GL_FOG_END, end);
	glFogfv(GL_FOG_COLOR, &color.r);
}

//-----------------------------------------------------------------------------
void GLRenderer::set_alpha_test(bool test)
{
	if (test)
	{
		glEnable(GL_ALPHA_TEST);
	}
	else
	{
		glDisable(GL_ALPHA_TEST);
	}
}

//-----------------------------------------------------------------------------
void GLRenderer::set_alpha_params(CompareFunction func, float ref)
{
	GLenum glFunc = GL::compare_function(func);

	glAlphaFunc(glFunc, ref);
}

//-----------------------------------------------------------------------------
void GLRenderer::set_shading_type(ShadingType type)
{
	GLenum glMode = GL_SMOOTH;

	if (type == ST_FLAT)
	{
		glMode = GL_FLAT;
	}

	glShadeModel(glMode);
}

//-----------------------------------------------------------------------------
void GLRenderer::set_polygon_mode(PolygonMode mode)
{
	GLenum glMode = GL::polygon_mode(mode);

	glPolygonMode(GL_FRONT_AND_BACK, glMode);
}

//-----------------------------------------------------------------------------
void GLRenderer::set_front_face(FrontFace face)
{
	GLenum glFace = GL_CCW;

	if (face == FF_CW)
	{
		glFace = GL_CW;
	}

	glFrontFace(glFace);
}

//-----------------------------------------------------------------------------
void GLRenderer::set_viewport_params(int32_t x, int32_t y, int32_t width, int32_t height)
{
	m_viewport[0] = x;
	m_viewport[1] = y;
	m_viewport[2] = width;
	m_viewport[3] = height;

	glViewport(x, y, width, height);
}

//-----------------------------------------------------------------------------
void GLRenderer::get_viewport_params(int32_t& x, int32_t& y, int32_t& width, int32_t& height)
{
	x = m_viewport[0];
	y = m_viewport[1];
	width = m_viewport[2];
	height = m_viewport[3];
}

//-----------------------------------------------------------------------------
void GLRenderer::set_scissor(bool scissor)
{
	if (scissor)
	{
		glEnable(GL_SCISSOR_TEST);
	}
	else
	{
		glDisable(GL_SCISSOR_TEST);
	}
}

//-----------------------------------------------------------------------------
void GLRenderer::set_scissor_params(int32_t x, int32_t y, int32_t width, int32_t height)
{
	m_scissor[0] = x;
	m_scissor[1] = y;
	m_scissor[2] = width;
	m_scissor[3] = height;

	glScissor(x, y, width, height);
}

//-----------------------------------------------------------------------------
void GLRenderer::get_scissor_params(int32_t& x, int32_t& y, int32_t& width, int32_t& height)
{
	x = m_scissor[0];
	y = m_scissor[1];
	width = m_scissor[2];
	height = m_scissor[3];
}

//-----------------------------------------------------------------------------
void GLRenderer::set_point_sprite(bool sprite)
{
	if (sprite)
	{
		glEnable(GL_POINT_SPRITE);
		glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);
	}
	else
	{
		glDisable(GL_POINT_SPRITE);
		glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_FALSE);
	}
}

//-----------------------------------------------------------------------------
void GLRenderer::set_point_size(float size)
{
	glPointSize(size);
}

//-----------------------------------------------------------------------------
void GLRenderer::set_point_params(float min, float max)
{
	glPointParameterf(GL_POINT_SIZE_MIN, min);
	glPointParameterf(GL_POINT_SIZE_MAX, max);
}

//-----------------------------------------------------------------------------
void GLRenderer::begin_frame()
{
	// Clear frame/depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Bind the default gpu program
	bind_gpu_program(m_default_gpu_program);
}

//-----------------------------------------------------------------------------
void GLRenderer::end_frame()
{
	glFinish();

	check_gl_errors();
}

//-----------------------------------------------------------------------------
Mat4 GLRenderer::get_matrix(MatrixType type) const
{
	return m_matrix[type];
}

//-----------------------------------------------------------------------------
void GLRenderer::set_matrix(MatrixType type, const Mat4& matrix)
{
	m_matrix[type] = matrix;

	switch (type)
	{
		case MT_VIEW:
		case MT_MODEL:
			glMatrixMode(GL_MODELVIEW);
			// Transformations must be listed in reverse order
			glLoadMatrixf((m_matrix[MT_VIEW] * m_matrix[MT_MODEL]).to_float_ptr());
			break;
		case MT_PROJECTION:
			glMatrixMode(GL_PROJECTION);
			glLoadMatrixf(m_matrix[MT_PROJECTION].to_float_ptr());
			break;
		case MT_TEXTURE:
			glMatrixMode(GL_TEXTURE);
			glLoadMatrixf(m_matrix[MT_TEXTURE].to_float_ptr());
			break;
		case MT_COLOR:
			//glMatrixMode(GL_COLOR);
			//glLoadMatrixf(m_matrix[MT_COLOR].to_float_ptr());
			break;
		default:
			break;
	}
}

//-----------------------------------------------------------------------------
void GLRenderer::bind_vertex_buffer(VertexBufferId vb) const
{
	assert(m_vertex_buffers_id_table.has(vb));

	const GLVertexBuffer& vertex_buffer = m_vertex_buffers[vb.index];

	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer.gl_object);

	switch (vertex_buffer.format)
	{
		case VF_XY_FLOAT_32:
		{
			glEnableVertexAttribArray(SA_VERTEX);
			glVertexAttribPointer(SA_VERTEX, 2, GL_FLOAT, GL_FALSE, 0, 0);
			break;
		}
		case VF_XYZ_FLOAT_32:
		{
			glEnableVertexAttribArray(SA_VERTEX);
			glVertexAttribPointer(SA_VERTEX, 3, GL_FLOAT, GL_FALSE, 0, 0);
			break;
		}
		case VF_UV_FLOAT_32:
		{
			glEnableVertexAttribArray(SA_COORDS);
			glVertexAttribPointer(SA_COORDS, 2, GL_FLOAT, GL_FALSE, 0, 0);
			break;
		}
		case VF_UVT_FLOAT_32:
		{
			glEnableVertexAttribArray(SA_COORDS);
			glVertexAttribPointer(SA_COORDS, 3, GL_FLOAT, GL_FALSE, 0, 0);
			break;
		}
		case VF_XYZ_NORMAL_FLOAT_32:
		{
			glEnableVertexAttribArray(SA_NORMAL);
			glVertexAttribPointer(SA_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, 0);
			break;
		}
		case VF_XYZ_UV_XYZ_NORMAL_FLOAT_32:
		{
			break;
		}
		default:
		{
			assert(0);
			break;
		}
	}
}

//-----------------------------------------------------------------------------
void GLRenderer::draw_triangles(IndexBufferId id) const
{
	assert(m_index_buffers_id_table.has(id));

	const GLIndexBuffer& index_buffer = m_index_buffers[id.index];

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer.gl_object);

	glDrawElements(GL_TRIANGLES, index_buffer.index_count, GL_UNSIGNED_SHORT, 0);
}

//-----------------------------------------------------------------------------
// void GLRenderer::bind_render_buffer(RenderBufferId id) const
// {
// 	assert(m_render_buffers_id_table.has(id));

// 	const GLRenderBuffer& render_buffer = m_render_buffers[id.index];
// }

//-----------------------------------------------------------------------------
void GLRenderer::set_light(uint32_t light, bool active)
{
	if (light >= (uint32_t) m_max_lights)
	{
		return;
	}

	if (active)
	{
		glEnable(GL_LIGHT0 + light);
	}
	else
	{
		glDisable(GL_LIGHT0 + light);
	}
}

//-----------------------------------------------------------------------------
void GLRenderer::set_light_params(uint32_t light, LightType type, const Vec3& position)
{
	static float pos[4] =
	{
		position.x,
		position.y,
		position.z,
		1.0f
	};

	if (type == LT_DIRECTION)
	{
		pos[3] = 0.0f;
	}

	glLightfv(GL_LIGHT0 + light, GL_POSITION, pos);
}

//-----------------------------------------------------------------------------
void GLRenderer::set_light_color(uint32_t light, const Color4& ambient, const Color4& diffuse, const Color4& specular)
{
	glLightfv(GL_LIGHT0 + light, GL_AMBIENT, ambient.to_float_ptr());
	glLightfv(GL_LIGHT0 + light, GL_DIFFUSE, diffuse.to_float_ptr());
	glLightfv(GL_LIGHT0 + light, GL_SPECULAR, specular.to_float_ptr());
}

//-----------------------------------------------------------------------------
void GLRenderer::set_light_attenuation(uint32_t light, float constant, float linear, float quadratic)
{
	glLightf(GL_LIGHT0 + light, GL_CONSTANT_ATTENUATION, constant);
	glLightf(GL_LIGHT0 + light, GL_LINEAR_ATTENUATION, linear);
	glLightf(GL_LIGHT0 + light, GL_QUADRATIC_ATTENUATION, quadratic);
}

//-----------------------------------------------------------------------------
void GLRenderer::draw_lines(const float* vertices, const float* colors, uint32_t count)
{
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);

	glVertexPointer(3, GL_FLOAT, 0, vertices);
	glColorPointer(4, GL_FLOAT, 0, colors);

	glDrawArrays(GL_LINES, 0, count);

	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
}

//-----------------------------------------------------------------------------
void GLRenderer::load_default_shaders()
{
	ResourceManager* resman = device()->resource_manager();

	// Load default vertex/pixel shaders
	m_default_vertex_shader = resman->load("default/default.vs");
	m_default_pixel_shader = resman->load("default/default.ps");

	// Wait for loading
	resman->flush();

	// Obtain resource data
	VertexShaderResource* vs = (VertexShaderResource*)resman->data(m_default_vertex_shader);
	PixelShaderResource* ps = (PixelShaderResource*)resman->data(m_default_pixel_shader);

	// Create and bind the default program
	m_default_gpu_program = create_gpu_program(vs->vertex_shader(), ps->pixel_shader());
}

//-----------------------------------------------------------------------------
void GLRenderer::unload_default_shaders()
{
	ResourceManager* resman = device()->resource_manager();

	destroy_gpu_program(m_default_gpu_program);

	resman->unload(m_default_pixel_shader);
	resman->unload(m_default_vertex_shader);
}

//-----------------------------------------------------------------------------
void GLRenderer::reload_default_shaders()
{
	ResourceManager* resman = device()->resource_manager();

	resman->reload(m_default_vertex_shader);
	resman->reload(m_default_pixel_shader);

	// Destroy old gpu program
	destroy_gpu_program(m_default_gpu_program);

	// Obtain resource data
	VertexShaderResource* vs = (VertexShaderResource*)resman->data(m_default_vertex_shader);
	PixelShaderResource* ps = (PixelShaderResource*)resman->data(m_default_pixel_shader);

	// Create and bind the new default program
	m_default_gpu_program = create_gpu_program(vs->vertex_shader(), ps->pixel_shader());
}

//-----------------------------------------------------------------------------
bool GLRenderer::activate_texture_unit(uint32_t unit)
{
	if (unit >= (uint32_t) m_max_texture_units)
	{
		return false;
	}

	glActiveTexture(GL_TEXTURE0 + unit);
	m_active_texture_unit = unit;

	return true;
}

//-----------------------------------------------------------------------------
GLint GLRenderer::find_gpu_program_uniform(GLuint program, const char* name) const
{
	GLint uniform = glGetUniformLocation(program, name);

	assert(uniform != -1);

	return uniform;
}

//-----------------------------------------------------------------------------
void GLRenderer::check_gl_errors() const
{
	GLenum error;

	while ((error = glGetError()))
	{
		switch (error)
		{
			case GL_INVALID_ENUM:
				Log::e("GLRenderer: GL_INVALID_ENUM");
				break;
			case GL_INVALID_VALUE:
				Log::e("GLRenderer: GL_INVALID_VALUE");
				break;
			case GL_INVALID_OPERATION:
				Log::e("GLRenderer: GL_INVALID_OPERATION");
				break;
			case GL_STACK_OVERFLOW:
				Log::e("GLRenderer: GL_STACK_OVERFLOW");
				break;
			case GL_STACK_UNDERFLOW:
				Log::e("GLRenderer: GL_STACK_UNDERFLOW");
				break;
			case GL_OUT_OF_MEMORY:
				Log::e("GLRenderer: GL_OUT_OF_MEMORY");
				break;
			case GL_TABLE_TOO_LARGE:
				Log::e("GLRenderer: GL_OUT_OF_MEMORY");
				break;
		}
	}
}

} // namespace crown

