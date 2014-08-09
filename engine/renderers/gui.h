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

#include "math_types.h"
#include "color4.h"
#include "render_world_types.h"

namespace crown
{

class RenderWorld;

/// Manages the rendering of GUI objects.
///
/// @ingroup Graphics
struct Gui
{
	Gui(RenderWorld& render_world, uint16_t width, uint16_t height);

	const GuiId id() const;
	void set_id(const GuiId id);

	Vector2 resolution() const;
	void move(const Vector2& pos);

	Vector2 screen_to_gui(const Vector2& pos);

	/// Draws a rectangle of size @a size at @a pos.
	/// @note Higher values of pos.z make the object appear in front of other objects.
	void draw_rectangle(const Vector3& pos, const Vector2& size, const Color4& color = Color4::WHITE);

	/// Draws an image with the given @a material.
	/// @note Higher values of pos.z make the object appear in front of other objects.
	void draw_image(const char* material, const Vector3& pos, const Vector2& size, const Color4& color = Color4::WHITE);

	/// Draws an image with the given @a material and @a uv0 and @a uv1 coordinates.
	/// @note Higher values of pos.z make the object appear in front of other objects.
	void draw_image_uv(const char* material, const Vector3& pos, const Vector2& size, const Vector2& uv0, const Vector2& uv1, const Color4& color = Color4::WHITE);

	/// Draws the text @a str with the given @a font and @a font_size.
	/// @note Higher values of pos.z make the object appear in front of other objects.
	void draw_text(const char* str, const char* font, uint32_t font_size, const Vector3& pos, const Color4& color = Color4::WHITE);

public:

	RenderWorld& m_render_world;
	GuiId m_id;

	uint16_t m_width;
	uint16_t m_height;

	Matrix4x4 m_projection;
	Matrix4x4 m_pose;
};

} // namespace crown
