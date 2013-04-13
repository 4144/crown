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

#pragma once

#include <GL/glew.h>
#include "Renderer.h"

#define MAX_TEXTURES 1024
#define MAX_TEXTURE_UNITS 8
#define MAX_MODEL_MATRIX_STACK_DEPTH 100

namespace crown
{

class TextureResource;

/// OpenGL texture
struct GLTexture
{
	TextureId			id;
	GLuint				texture_object;
	TextureResource*	texture_resource;
};

/// OpenGL renderer
class GLRenderer : public Renderer
{
public:

						GLRenderer();
						~GLRenderer();

	void				begin_frame();
	void				end_frame();

	void				set_clear_color(const Color4& color);
	void				set_viewport(const Rect& absArea);
	void				set_material_params(const Color4& ambient, const Color4& diffuse, const Color4& specular, const Color4& emission, int32_t shininess);
	void				set_lighting(bool lighting);
	void				set_ambient_light(const Color4& color);

	void				set_texturing(uint32_t unit, bool texturing);
	void				set_texture(uint32_t unit, TextureId texture);
	void				set_texture_mode(uint32_t unit, TextureMode mode, const Color4& blendColor);
	void				set_texture_wrap(uint32_t unit, TextureWrap wrap);
	void				set_texture_filter(uint32_t unit, TextureFilter filter);

	void				set_light(uint32_t light, bool active);
	void				set_light_params(uint32_t light, LightType type, const Vec3& position);
	void				set_light_color(uint32_t light, const Color4& ambient, const Color4& diffuse, const Color4& specular);
	void				set_light_attenuation(uint32_t light, float constant, float linear, float quadratic);

	void				set_backface_culling(bool culling);

	void				set_separate_specular_color(bool separate);

	void				set_depth_test(bool test);
	void				set_depth_write(bool write);
	void				set_depth_func(CompareFunction func);

	void				set_rescale_normals(bool rescale);

	void				set_blending(bool blending);
	void				set_blending_params(BlendEquation equation, BlendFunction src, BlendFunction dst, const Color4& color);
	void				set_color_write(bool write);

	void				set_fog(bool fog);
	void				set_fog_params(FogMode mode, float density, float start, float end, const Color4& color);

	void				set_alpha_test(bool test);
	void				set_alpha_params(CompareFunction func, float ref);

	void				set_shading_type(ShadingType type);
	void				set_polygon_mode(PolygonMode mode);
	void				set_front_face(FrontFace face);

	void				set_viewport_params(int32_t x, int32_t y, int32_t width, int32_t height);

	void				set_scissor(bool scissor);
	void				set_scissor_params(int32_t x, int32_t y, int32_t width, int32_t height);

	void				set_point_sprite(bool sprite);
	void				set_point_size(float size);
	void				set_point_params(float min, float max);

	Mat4				get_matrix(MatrixType type) const;
	void				set_matrix(MatrixType type, const Mat4& matrix);

	void				push_matrix();
	void				pop_matrix();

	void				select_matrix(MatrixType type);

	void				render_vertex_index_buffer(const VertexBuffer* vertices, const IndexBuffer* indices);
	void				render_point_buffer(const VertexBuffer* buffer);

	void				set_scissor_box(uint32_t x, uint32_t y, uint32_t width, uint32_t height);
	void				get_scissor_box(uint32_t& x, uint32_t& y, uint32_t& width, uint32_t& height);
	void				draw_rectangle(const Point2& position, const Point2& dimensions, int32_t drawMode,
														 const Color4& borderColor, const Color4& fillColor);

	void				add_debug_line(const Vec3& start, const Vec3& end, const Color4& color);
	void				draw_debug_lines();

	TextureId			load_texture(TextureResource* texture);
	void				unload_texture(TextureResource* texture);
	TextureId			reload_texture(TextureResource* old_texture, TextureResource* new_texture);

	bool				activate_texture_unit(uint32_t unit);		//!< Activates a texture unit and returns true if succes
	bool				activate_light(uint32_t light);

	void				check_gl_errors();

private:

	// Matrices
	Mat4				mMatrix[MT_COUNT];

	Mat4				mModelMatrixStack[MAX_MODEL_MATRIX_STACK_DEPTH];
	uint32_t			mModelMatrixStackIndex;

	// Limits
	int32_t				mMaxLights;
	int32_t				mMaxTextureSize;
	int32_t				mMaxTextureUnits;
	int32_t				mMaxVertexIndices;
	int32_t				mMaxVertexVertices;

	float				mMaxAnisotropy;
	float				mMinMaxPointSize[2];
	float				mMinMaxLineWidth[2];

	// Texture management
	uint32_t			m_texture_count;
	GLTexture			m_textures[MAX_TEXTURES];

	uint32_t			mActiveTextureUnit;
	GLuint				mTextureUnit[MAX_TEXTURE_UNITS];
	GLenum				mTextureUnitTarget[MAX_TEXTURE_UNITS];

	// Debug lines
	struct DebugLinesData
	{
		Vec3	start;
		Color4	c1;
		Vec3	end;
		Color4	c2;
	};

	uint32_t				mLinesCount;
	DebugLinesData		mLinesData[1024];

	friend class TextureResource;
};

} // namespace crown

