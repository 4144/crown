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

#include <cstring>
#include <inttypes.h>

#include "Allocator.h"
#include "Filesystem.h"
#include "Hash.h"
#include "JSONParser.h"
#include "SpriteResource.h"
#include "StringUtils.h"

namespace crown
{
namespace sprite_resource
{

//-----------------------------------------------------------------------------
struct FrameData
{
	float x0, y0;
	float x1, y1;

	float scale_x, scale_y;
	float offset_x, offset_y;
};

//-----------------------------------------------------------------------------
void parse_frame(JSONElement frame, List<StringId32>& names, List<FrameData>& regions)
{
	JSONElement name = frame.key("name");
	JSONElement region = frame.key("region");
	JSONElement offset = frame.key("offset");
	JSONElement scale = frame.key("scale");

	DynamicString frame_name;
	name.to_string(frame_name);

	StringId32 name_hash = hash::murmur2_32(frame_name.c_str(), frame_name.length(), 0);
	FrameData fd;
	fd.x0 = region[0].to_float();
	fd.y0 = region[1].to_float();
	fd.x1 = region[2].to_float();
	fd.y1 = region[3].to_float();
	fd.offset_x = offset[0].to_float();
	fd.offset_y = offset[1].to_float();
	fd.scale_x = scale[0].to_float();
	fd.scale_y = scale[1].to_float();

	names.push_back(name_hash);
	regions.push_back(fd);
}

//-----------------------------------------------------------------------------
void compile(Filesystem& fs, const char* resource_path, File* out_file)
{
	File* file = fs.open(resource_path, FOM_READ);
	char* buf = (char*)default_allocator().allocate(file->size());
	file->read(buf, file->size());

	JSONParser json(buf);
	JSONElement root = json.root();

	float width;
	float height;
	List<StringId32>		m_names(default_allocator());
	List<FrameData> 		m_regions(default_allocator());
	List<float>				m_vertices(default_allocator());
	List<uint16_t>			m_indices(default_allocator());

	// Read width/height
	width = root.key("width").to_float();
	height = root.key("height").to_float();

	// Read frames
	JSONElement frames = root.key("frames");
	uint32_t num_frames = frames.size();
	for (uint32_t i = 0; i < num_frames; i++)
	{
		parse_frame(frames[i], m_names, m_regions);
	}

	uint32_t num_idx = 0;
	for (uint32_t i = 0; i < num_frames; i++)
	{
		const FrameData& fd = m_regions[i];

		// Compute uv coords
		const float u0 = fd.x0;
		const float v0 = fd.y0;
		const float u1 = fd.x0 + fd.x1;
		const float v1 = fd.y0 + fd.y1;

		const float aspect = (fd.x1 * width) / (fd.y1 * height);

		// Compute positions
		const float w = aspect;
		const float h = 1;

		const float x0 = fd.scale_x * (-w * 0.5) + fd.offset_x;
		const float y0 = fd.scale_y * (-h * 0.5) + fd.offset_y;
		const float x1 = fd.scale_x * ( w * 0.5) + fd.offset_x;
		const float y1 = fd.scale_y * ( h * 0.5) + fd.offset_y;

		m_vertices.push_back(x0); m_vertices.push_back(y0); // position
		m_vertices.push_back(u0); m_vertices.push_back(v0); // uv

		m_vertices.push_back(x1); m_vertices.push_back(y0); // position
		m_vertices.push_back(u1); m_vertices.push_back(v0); // uv

		m_vertices.push_back(x1); m_vertices.push_back(y1); // position
		m_vertices.push_back(u1); m_vertices.push_back(v1); // uv

		m_vertices.push_back(x0); m_vertices.push_back(y1); // position
		m_vertices.push_back(u0); m_vertices.push_back(v1); // uv

		m_indices.push_back(num_idx); m_indices.push_back(num_idx + 1); m_indices.push_back(num_idx + 2);
		m_indices.push_back(num_idx); m_indices.push_back(num_idx + 2); m_indices.push_back(num_idx + 3);
		num_idx += 4;
	}

	fs.close(file);
	default_allocator().deallocate(buf);

	SpriteHeader h;
	h.num_frames = m_names.size();
	h.num_vertices = m_vertices.size() / 4; // 4 components per vertex
	h.num_indices = m_indices.size();

	uint32_t offt = sizeof(SpriteHeader);
	/*h.frame_names_offset    = offt*/; offt += sizeof(StringId32) * h.num_frames;
	h.vertices_offset = offt; offt += sizeof(float) * m_vertices.size();
	h.indices_offset = offt; offt += sizeof(uint16_t) * m_indices.size();

	out_file->write((char*) &h, sizeof(SpriteHeader));
	out_file->write((char*) m_names.begin(), sizeof(StringId32) * m_names.size());
	out_file->write((char*) m_vertices.begin(), sizeof(float) * m_vertices.size());
	out_file->write((char*) m_indices.begin(), sizeof(uint16_t) * m_indices.size());
}

} // namespace sprite_resource
} // namespace crown