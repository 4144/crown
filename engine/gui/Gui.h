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

#include "Matrix4x4.h"
#include "IdTable.h"
#include "IdArray.h"
#include "PoolAllocator.h"
#include "Vector2.h"
#include "Resource.h"
#include "Config.h"

namespace crown
{

typedef Id UniformId;

typedef Id GuiId;
typedef Id GuiRectId;
typedef Id GuiTriangleId;
typedef Id GuiImageId;
typedef Id GuiTextId;

struct Renderer;
struct RenderWorld;
struct GuiResource;
struct GuiRect;
struct GuiTriangle;
struct GuiImage;
struct GuiText;
struct Vector3;
struct Vector2;
struct Color4;
struct FontResource;

struct Gui
{
						Gui(RenderWorld& render_world, GuiResource* gr, Renderer& r);
						~Gui();

	const GuiId 		id() const;
	void				set_id(const GuiId id);

	Vector2				resolution() const;
	void				move(const Vector3& pos);

	void				show();
	void				hide();

	GuiRectId			create_rect(const Vector3& pos, const Vector2& size, const Color4& color);
	void				update_rect(GuiRectId id, const Vector3& pos, const Vector2& size, const Color4& color);
	void				destroy_rect(GuiRectId id);

	GuiTriangleId		create_triangle(const Vector2& p1, const Vector2& p2, const Vector2& p3, const Color4& color);
	void				update_triangle(GuiTriangleId id, const Vector2& p1, const Vector2& p2, const Vector2& p3, const Color4& color);
	void				destroy_triangle(GuiTriangleId id);

	GuiImageId			create_image(ResourceId material, const Vector3& pos, const Vector2& size);
	void				update_image(GuiImageId id, const Vector3& pos, const Vector2& size);
	void				destroy_image(GuiImageId);

	GuiTextId			create_text(const char* str, const FontResource* font, uint32_t font_size, const Vector3& pos);
	void				update_text(GuiTextId id, const char* str, uint32_t font_size, const Vector3& pos);
	void				destroy_text(GuiTextId id);

	void				render(UniformId font, UniformId albedo);

private:

	// FIXME FIXME FIXME -- tmp way
	void 				create_gfx();
	void 				destroy_gfx();

public:

	RenderWorld&		m_render_world;
	const GuiResource*	m_resource;
	Renderer&			m_r;
	GuiId				m_id;

	Matrix4x4			m_projection;
	Matrix4x4			m_pose;
	Vector2				m_resolution;

	bool				m_visible;

	PoolAllocator		m_rect_pool;
	PoolAllocator		m_triangle_pool;
	PoolAllocator		m_image_pool;
	PoolAllocator		m_text_pool;

	IdArray<CE_MAX_GUI_RECTS, GuiRect*> m_rects;
	IdArray<CE_MAX_GUI_TRIANGLES, GuiTriangle*> m_triangles;
	IdArray<CE_MAX_GUI_IMAGES, GuiImage*> m_images;
	IdArray<CE_MAX_GUI_TEXTS, GuiText*> m_texts;
};

} // namespace crown