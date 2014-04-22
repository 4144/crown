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

#include "Gui.h"
#include "Assert.h"
#include "Renderer.h"
#include "Vector3.h"
#include "Vector2.h"
#include "GuiResource.h"
#include "RenderWorld.h"
#include "RendererTypes.h"
#include "Vector3.h"
#include "Vector2.h"
#include "Color4.h"
#include "FontResource.h"
#include "Device.h"
#include "OsWindow.h"
#include "ResourceManager.h"
#include "TextureResource.h"
#include "MaterialResource.h"

namespace crown
{

using namespace matrix4x4;

//-------------------------------------------------------------------------
struct VertexData
{
	float x;
	float y;
	float u;
	float v;
};

//-------------------------------------------------------------------------
struct IndexData
{
	uint16_t a;
	uint16_t b;
};

#define UTF8_ACCEPT 0

//-------------------------------------------------------------------------
static const uint8_t s_utf8d[364] =
{
	// The first part of the table maps bytes to character classes that
	// to reduce the size of the transition table and create bitmasks.
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,  9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,
	7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,  7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
	8,8,2,2,2,2,2,2,2,2,2,2,2,2,2,2,  2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
	10,3,3,3,3,3,3,3,3,3,3,3,3,4,3,3, 11,6,6,6,5,8,8,8,8,8,8,8,8,8,8,8,

	// The second part is a transition table that maps a combination
	// of a state of the automaton and a character class to a state.
	 0,12,24,36,60,96,84,12,12,12,48,72, 12,12,12,12,12,12,12,12,12,12,12,12,
	12, 0,12,12,12,12,12, 0,12, 0,12,12, 12,24,12,12,12,12,12,24,12,24,12,12,
	12,12,12,12,12,12,12,24,12,12,12,12, 12,24,12,12,12,12,12,12,12,24,12,12,
	12,12,12,12,12,12,12,36,12,36,12,12, 12,36,12,12,12,12,12,36,12,36,12,12,
	12,36,12,12,12,12,12,12,12,12,12,12
};

//-------------------------------------------------------------------------
static uint32_t utf8_decode(uint32_t* state, uint32_t* code_point, uint8_t character)
{
	uint32_t byte = character;
	uint32_t type = s_utf8d[byte];

	*code_point = (*state != UTF8_ACCEPT) ? (byte & 0x3fu) | (*code_point << 6) : (0xff >> type) & (byte);
	*state = s_utf8d[256 + *state + type];

	return *state;
}

//-----------------------------------------------------------------------------
Gui::Gui(RenderWorld& render_world, uint16_t width, uint16_t height)
	: m_render_world(render_world)
	, m_width(width)
	, m_height(height)
	, m_visible(true)
{
	set_orthographic_rh(m_projection, 0, width, height, 0, -0.01f, 100.0f);
}

//-----------------------------------------------------------------------------
const GuiId Gui::id() const
{
	return m_id;
}

//-----------------------------------------------------------------------------
void Gui::set_id(const GuiId id)
{
	m_id = id;
}

//-----------------------------------------------------------------------------
Vector2 Gui::resolution() const
{
	return Vector2(m_width, m_height);
}

//-----------------------------------------------------------------------------
void Gui::move(const Vector3& pos)
{
	set_identity(m_pose);
	set_translation(m_pose, pos);
}

//-----------------------------------------------------------------------------
void Gui::show()
{
	m_visible = true;
}

//-----------------------------------------------------------------------------
void Gui::hide()
{
	m_visible = false;
}

//-----------------------------------------------------------------------------
void Gui::draw_rectangle(const Vector3& pos, const Vector2& size, const Color4& color)
{
	Renderer* r = device()->renderer();
	TransientVertexBuffer tvb;
	TransientIndexBuffer tib;

	r->reserve_transient_vertex_buffer(&tvb, 4, VertexFormat::P2);
	r->reserve_transient_index_buffer(&tib, 6);

	float* verts = (float*) tvb.data;
	verts[0] = pos.x;
	verts[1] = pos.y;

	verts[2] = pos.x + size.x;
	verts[3] = pos.y;

	verts[4] = pos.x + size.x;
	verts[5] = pos.y - size.y;

	verts[6] = pos.x;
	verts[7] = pos.y - size.y;

	uint16_t* inds = (uint16_t*) tib.data;
	inds[0] = 0;
	inds[1] = 1;
	inds[2] = 2;
	inds[3] = 0;
	inds[4] = 2;
	inds[5] = 3;

	r->set_layer_view(1, matrix4x4::IDENTITY);
	r->set_layer_projection(1, m_projection);
	r->set_layer_viewport(1, 0, 0, m_width, m_height);
	r->set_state(STATE_COLOR_WRITE
					| STATE_CULL_CW
					| STATE_BLEND_EQUATION_ADD 
					| STATE_BLEND_FUNC(STATE_BLEND_FUNC_SRC_ALPHA, STATE_BLEND_FUNC_ONE_MINUS_SRC_ALPHA));

	r->set_program(render_world_globals::default_program());
	r->set_uniform(render_world_globals::default_color_uniform(), UniformType::FLOAT_4, color4::to_float_ptr(color), 1);
	r->set_vertex_buffer(tvb);
	r->set_index_buffer(tib);
	r->commit(1);
}

//-----------------------------------------------------------------------------
void Gui::draw_image(const char* material, const Vector3& pos, const Vector2& size, const Color4& color)
{
	Renderer* r = device()->renderer();
	TransientVertexBuffer tvb;
	TransientIndexBuffer tib;

	r->reserve_transient_vertex_buffer(&tvb, 4, VertexFormat::P2_T2);
	r->reserve_transient_index_buffer(&tib, 6);

	float* verts = (float*) tvb.data;
	verts[0] = pos.x;
	verts[1] = pos.y;
	verts[2] = 0;
	verts[3] = 0;

	verts[4] = pos.x + size.x;
	verts[5] = pos.y;
	verts[6] = 1;
	verts[7] = 0;

	verts[8] = pos.x + size.x;
	verts[9] = pos.y - size.y;
	verts[10] = 1;
	verts[11] = 1;

	verts[12] = pos.x;
	verts[13] = pos.y - size.y;
	verts[14] = 0;
	verts[15] = 1;

	uint16_t* inds = (uint16_t*) tib.data;
	inds[0] = 0;
	inds[1] = 1;
	inds[2] = 2;
	inds[3] = 0;
	inds[4] = 2;
	inds[5] = 3;

	const MaterialResource* mr = (MaterialResource*) device()->resource_manager()->lookup("material", material);
	const TextureResource* tr = (TextureResource*) device()->resource_manager()->data(mr->get_texture_layer(0));

	r->set_layer_view(1, matrix4x4::IDENTITY);
	r->set_layer_projection(1, m_projection);
	r->set_layer_viewport(1, 0, 0, m_width, m_height);
	r->set_state(STATE_COLOR_WRITE
					| STATE_CULL_CW
					| STATE_BLEND_EQUATION_ADD 
					| STATE_BLEND_FUNC(STATE_BLEND_FUNC_SRC_ALPHA, STATE_BLEND_FUNC_ONE_MINUS_SRC_ALPHA));

	r->set_program(render_world_globals::default_texture_program());
	r->set_texture(0, render_world_globals::default_albedo_uniform(), tr->texture(),
					TEXTURE_FILTER_LINEAR | TEXTURE_WRAP_U_CLAMP_REPEAT | TEXTURE_WRAP_V_CLAMP_REPEAT);
	r->set_uniform(render_world_globals::default_color_uniform(), UniformType::FLOAT_4, color4::to_float_ptr(color), 1);
	r->set_vertex_buffer(tvb);
	r->set_index_buffer(tib);
	r->commit(1);
}

//-----------------------------------------------------------------------------
void Gui::draw_text(const char* str, const char* font, uint32_t font_size, const Vector3& pos, const Color4& color)
{
	Renderer* r = device()->renderer();

	const FontResource* resource = (FontResource*) device()->resource_manager()->lookup("font", font);
	Vector2 m_pen;

	const float scale = ((float)font_size / (float)resource->font_size());
	const uint32_t str_len = string::strlen(str);

	TransientVertexBuffer vb;
	TransientIndexBuffer ib;

	r->reserve_transient_vertex_buffer(&vb, 4 * str_len, VertexFormat::P2_T2);
	r->reserve_transient_index_buffer(&ib, 6 * str_len);

	uint16_t index = 0;
	float x_pen_advance = 0.0f;
	float y_pen_advance = 0.0f;

	uint32_t state = 0;
	uint32_t code_point = 0;
	for (uint32_t i = 0; i < str_len; i++)
	{
		switch (str[i])
		{
			case '\n':
			{
				x_pen_advance = 0.0f;
				y_pen_advance += resource->font_size();
				continue;
			}
			case '\t':
			{
				x_pen_advance += font_size * 4;
				continue;
			}
		}
		
		if (utf8_decode(&state, &code_point, str[i]) == UTF8_ACCEPT)
		{
			FontGlyphData g = resource->get_glyph(code_point);

			// Set pen position
			m_pen.x = pos.x + g.x_offset;
			m_pen.y = pos.y + (g.height - g.y_offset);

			// Position coords
			const float x0 = (m_pen.x + x_pen_advance) * scale;
			const float y0 = (m_pen.y + y_pen_advance) * scale;
			const float x1 = (m_pen.x + g.width + x_pen_advance) * scale;
			const float y1 = (m_pen.y - g.height + y_pen_advance) * scale;

			// Texture coords
			const float u0 = (float) g.x / 512;
			const float v0 = (float) g.y / 512;
			const float u1 = u0 + ((float) g.width) / 512;
			const float v1 = v0 - ((float) g.height) / 512;

			// Fill vertex buffer
			(*(VertexData*)(vb.data)).x		= x0;
			(*(VertexData*)(vb.data)).y		= y0;
			(*(VertexData*)(vb.data)).u		= u0;
			(*(VertexData*)(vb.data)).v		= v1;
			vb.data += sizeof(VertexData);

			(*(VertexData*)(vb.data)).x		= x1;
			(*(VertexData*)(vb.data)).y		= y0;
			(*(VertexData*)(vb.data)).u		= u1; 
			(*(VertexData*)(vb.data)).v		= v1;
			vb.data += sizeof(VertexData);

			(*(VertexData*)(vb.data)).x		= x1;
			(*(VertexData*)(vb.data)).y		= y1;
			(*(VertexData*)(vb.data)).u		= u1;
			(*(VertexData*)(vb.data)).v		= v0;
			vb.data += sizeof(VertexData);

			(*(VertexData*)(vb.data)).x		= x0;
			(*(VertexData*)(vb.data)).y		= y1;
			(*(VertexData*)(vb.data)).u		= u0;
			(*(VertexData*)(vb.data)).v		= v0;
			vb.data += sizeof(VertexData);

			// Fill index buffer
			(*(IndexData*)(ib.data)).a		= index;
			(*(IndexData*)(ib.data)).b		= index + 1;
			ib.data += sizeof(IndexData);

			(*(IndexData*)(ib.data)).a		= index + 2;
			(*(IndexData*)(ib.data)).b		= index;
			ib.data += sizeof(IndexData);

			(*(IndexData*)(ib.data)).a		= index + 2;
			(*(IndexData*)(ib.data)).b		= index + 3;
			ib.data += sizeof(IndexData);

			// Advance pen position
			x_pen_advance += g.x_advance;

			index += 4;
		}
	}

	const MaterialResource* mr = (MaterialResource*) device()->resource_manager()->data(resource->material());
	const TextureResource* tr = (TextureResource*) device()->resource_manager()->data(mr->get_texture_layer(0));

	r->set_layer_view(1, matrix4x4::IDENTITY);
	r->set_layer_projection(1, m_projection);
	r->set_layer_viewport(1, 0, 0, 1000, 625);
	r->set_state(STATE_COLOR_WRITE
					| STATE_CULL_CW
					| STATE_BLEND_EQUATION_ADD 
					| STATE_BLEND_FUNC(STATE_BLEND_FUNC_SRC_ALPHA, STATE_BLEND_FUNC_ONE_MINUS_SRC_ALPHA));

	r->set_program(render_world_globals::default_font_program());
	r->set_texture(0, render_world_globals::default_font_uniform(), tr->texture(),
					TEXTURE_FILTER_LINEAR | TEXTURE_WRAP_U_CLAMP_REPEAT | TEXTURE_WRAP_V_CLAMP_REPEAT);
	r->set_uniform(render_world_globals::default_color_uniform(), UniformType::FLOAT_4, color4::to_float_ptr(color), 1);
	r->set_vertex_buffer(vb);
	r->set_index_buffer(ib);
	r->commit(1);
}

} // namespace crown
