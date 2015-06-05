/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "math_types.h"
#include "color4.h"
#include "render_world_types.h"
#include "material.h"
#include <bgfx.h>

namespace crown
{

class RenderWorld;

/// Manages the rendering of GUI objects.
///
/// @ingroup Graphics
struct Gui
{
	Gui(uint16_t width, uint16_t height, const char* material);

	const GuiId id() const;
	void set_id(const GuiId id);

	Vector2 resolution() const;
	void move(const Vector2& pos);

	Vector2 screen_to_gui(const Vector2& pos);

	/// Draws a rectangle of size @a size at @a pos.
	/// @note Higher values of pos.z make the object appear in front of other objects.
	void draw_rectangle(const Vector3& pos, const Vector2& size, const Color4& color = color4::WHITE);

	/// Draws an image with the given @a material.
	/// @note Higher values of pos.z make the object appear in front of other objects.
	void draw_image(const char* material, const Vector3& pos, const Vector2& size, const Color4& color = color4::WHITE);

	/// Draws an image with the given @a material and @a uv0 and @a uv1 coordinates.
	/// @note Higher values of pos.z make the object appear in front of other objects.
	void draw_image_uv(const char* material, const Vector3& pos, const Vector2& size, const Vector2& uv0, const Vector2& uv1, const Color4& color = color4::WHITE);

	/// Draws the text @a str with the given @a font and @a font_size.
	/// @note Higher values of pos.z make the object appear in front of other objects.
	void draw_text(const char* str, const char* font, uint32_t font_size, const Vector3& pos, const Color4& color = color4::WHITE);

public:

	GuiId m_id;
	uint16_t m_width;
	uint16_t m_height;
	typedef Id MaterialId;
	MaterialId m_material;
	Matrix4x4 m_projection;
	Matrix4x4 m_pose;

	static void init();

private:

	static bgfx::VertexDecl s_pos_col;
	static bgfx::VertexDecl s_pos_col_tex;
};

} // namespace crown
