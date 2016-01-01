/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "font_resource.h"
#include "allocator.h"
#include "filesystem.h"
#include "string_utils.h"
#include "compile_options.h"
#include "sjson.h"
#include "map.h"

namespace crown
{
namespace font_resource
{
	void parse_glyph(const char* json, FontGlyphData& glyph)
	{
		TempAllocator512 ta;
		JsonObject obj(ta);
		sjson::parse(json, obj);

		glyph.id        = sjson::parse_int(obj["id"]);
		glyph.x         = sjson::parse_int(obj["x"]);
		glyph.y         = sjson::parse_int(obj["y"]);
		glyph.width     = sjson::parse_int(obj["width"]);
		glyph.height    = sjson::parse_int(obj["height"]);
		glyph.x_offset  = sjson::parse_float(obj["x_offset"]);
		glyph.y_offset  = sjson::parse_float(obj["y_offset"]);
		glyph.x_advance = sjson::parse_float(obj["x_advance"]);
	}

	void compile(const char* path, CompileOptions& opts)
	{
		Buffer buf = opts.read(path);

		TempAllocator4096 ta;
		JsonObject object(ta);
		JsonArray glyphs(ta);

		sjson::parse(buf, object);
		sjson::parse_array(object["glyphs"], glyphs);

		const uint32_t count      = sjson::parse_int(object["count"]);
		const uint32_t size       = sjson::parse_int(object["size"]);
		const uint32_t font_size  = sjson::parse_int(object["font_size"]);
		const uint32_t num_glyphs = array::size(glyphs);

		Array<FontGlyphData> m_glyphs(default_allocator());

		for (uint32_t i = 0; i < num_glyphs; ++i)
		{
			FontGlyphData data;
			parse_glyph(glyphs[i], data);
			array::push_back(m_glyphs, data);
		}

		// Write
		FontResource fr;
		fr.version      = FONT_VERSION;
		fr.num_glyphs   = array::size(m_glyphs);
		fr.texture_size = size;
		fr.font_size    = font_size;

		opts.write(fr.version);
		opts.write(fr.num_glyphs);
		opts.write(fr.texture_size);
		opts.write(fr.font_size);

		for (uint32_t i = 0; i < array::size(m_glyphs); ++i)
		{
			opts.write(m_glyphs[i].id);
			opts.write(m_glyphs[i].x);
			opts.write(m_glyphs[i].y);
			opts.write(m_glyphs[i].width);
			opts.write(m_glyphs[i].height);
			opts.write(m_glyphs[i].x_offset);
			opts.write(m_glyphs[i].y_offset);
			opts.write(m_glyphs[i].x_advance);
		}
	}

	void* load(File& file, Allocator& a)
	{
		const uint32_t file_size = file.size();
		void* res = a.allocate(file_size);
		file.read(res, file_size);
		CE_ASSERT(*(uint32_t*)res == FONT_VERSION, "Wrong version");
		return res;
	}

	void unload(Allocator& allocator, void* resource)
	{
		allocator.deallocate(resource);
	}

	uint32_t num_glyphs(const FontResource* fr)
	{
		return fr->num_glyphs;
	}

	uint32_t texture_size(const FontResource* fr)
	{
		return fr->texture_size;
	}

	uint32_t font_size(const FontResource* fr)
	{
		return fr->font_size;
	}

	const FontGlyphData* get_glyph(const FontResource* fr, uint32_t i)
	{
		CE_ASSERT(i < num_glyphs(fr), "Index out of bounds");

		FontGlyphData* begin = (FontGlyphData*)((char*)fr + sizeof(FontResource));

		for (uint32_t i = 0; i < num_glyphs(fr); i++)
		{
			if (begin[i].id == i)
				return &begin[i];
		}

		CE_FATAL("Glyph not found");
		return NULL;
	}
} // namespace font_resource
} // namespace crown
