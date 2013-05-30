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

#include "TGACompiler.h"
#include "DiskFile.h"
#include "PixelFormat.h"
#include "Resource.h"
#include "Log.h"

namespace crown
{

//-----------------------------------------------------------------------------
TGACompiler::TGACompiler(const char* root_path, const char* dest_path) :
	Compiler(root_path, dest_path, TEXTURE_TYPE),
	m_image_format(PF_UNKNOWN),
	m_image_channels(0),
	m_image_size(0),
	m_image_data(NULL)
{
	memset(&m_tga_header, 0, sizeof(TGAHeader));
}

//-----------------------------------------------------------------------------
TGACompiler::~TGACompiler()
{
	cleanup_impl();
}

//-----------------------------------------------------------------------------
size_t TGACompiler::read_header_impl(DiskFile* in_file)
{
	// Read the header
	in_file->read(&m_tga_header, sizeof(TGAHeader));

	// Skip TGA ID
	in_file->skip(m_tga_header.id_length);

	return sizeof(TGAHeader) + m_tga_header.id_length;
}

//-----------------------------------------------------------------------------
size_t TGACompiler::read_resource_impl(DiskFile* in_file)
{
	// Compute color channels	
	m_image_channels = m_tga_header.pixel_depth / 8;
	
	// Compute image size
	m_image_size = m_tga_header.width * m_tga_header.height;

	// Select the appropriate pixel format and allocate
	// resource data based on tga size and channels
	switch (m_image_channels)
	{
		case 2:
		case 3:
		{
			m_image_format = PF_RGB_8;
			m_image_data = (uint8_t*)default_allocator().allocate(m_image_size * 3 * sizeof(uint8_t));

			break;
		}
		case 4:
		{
			m_image_format = PF_RGBA_8;
			m_image_data = (uint8_t*)default_allocator().allocate(m_image_size * m_image_channels * sizeof(uint8_t));
			
			break;
		}
		default:
		{
			Log::e("Unable to determine TGA channels.");
			return 0;
		}
	}

	// Determine image type (compressed/uncompressed) and call proper function to load TGA
	switch (m_tga_header.image_type)
	{
		case 0:
		{
			Log::e("Fatal: The resource does not contain image data.");
			return 0;
		}
		case 2:
		{
			load_uncompressed(in_file);
			break;
		}

		case 10:
		{
			load_compressed(in_file);
			break;
		}

		default:
		{
			Log::e("Fatal: Image type not supported.");
			return 0;
		}
	}

	// Return the total resource size
	return m_image_size * m_image_channels + sizeof(PixelFormat) + sizeof(uint16_t) + sizeof(uint16_t);
}

//-----------------------------------------------------------------------------
void TGACompiler::write_header_impl(DiskFile* out_file)
{
	// Write the texture header
	out_file->write(&m_image_format, sizeof(PixelFormat));
	out_file->write(&m_tga_header.width, sizeof(uint16_t));
	out_file->write(&m_tga_header.height, sizeof(uint16_t));
}

//-----------------------------------------------------------------------------
void TGACompiler::write_resource_impl(DiskFile* out_file)
{
	// Write out the data
	out_file->write(m_image_data, m_image_size * m_image_channels);
}

//-----------------------------------------------------------------------------
void TGACompiler::cleanup_impl()
{
	if (m_image_data)
	{
		default_allocator().deallocate(m_image_data);
		m_image_data = NULL;
	}
}

//-----------------------------------------------------------------------------
void TGACompiler::load_uncompressed(DiskFile* in_file)
{
	uint64_t size = m_tga_header.width * m_tga_header.height;

	if (m_image_channels == 2)
	{
		int32_t j = 0;

		for (uint64_t i = 0; i < size * m_image_channels; i++)
		{
			uint16_t pixel_data;
			
			in_file->read(&pixel_data, sizeof(pixel_data));
			
			m_image_data[j + 0] = (pixel_data & 0x7c) >> 10;
			m_image_data[j + 1] = (pixel_data & 0x3e) >> 5;
			m_image_data[j + 2] = (pixel_data & 0x1f);
			
			j += 3;
		}
	}
	else
	{
		in_file->read(m_image_data, (size_t)(size * m_image_channels));

		swap_red_blue();
	}
}

//-----------------------------------------------------------------------------
void TGACompiler::load_compressed(DiskFile* in_file)
{
	uint8_t rle_id = 0;
	uint32_t i = 0;
	uint32_t colors_read = 0;
	uint64_t size = m_tga_header.width * m_tga_header.height;

	// Can't be more than 4 channels
	uint8_t colors[4];

	while (i < size)
	{
		in_file->read(&rle_id, sizeof(uint8_t));

		// If MSB == 1
		if (rle_id & 0x80)
		{
			rle_id -= 127;
			
			in_file->read(&colors, m_image_channels);

			while (rle_id)
			{
				m_image_data[colors_read + 0] = colors[2];
				m_image_data[colors_read + 1] = colors[1];
				m_image_data[colors_read + 2] = colors[0];

				if (m_image_channels == 4)
				{
					m_image_data[colors_read + 3] = colors[3];
				}

				rle_id--;
				colors_read += m_image_channels;
				i++;
			}
		}
		else
		{
			rle_id++;

			while (rle_id)
			{
				in_file->read(colors, m_image_channels);
				
				m_image_data[colors_read + 0] = colors[2];
				m_image_data[colors_read + 1] = colors[1];
				m_image_data[colors_read + 2] = colors[0];

				if (m_image_channels == 4)
				{
					m_image_data[colors_read + 3] = colors[3];
				}

				rle_id--;
				colors_read += m_image_channels;
				i++;
			}
		}
	}
}

//-----------------------------------------------------------------------------
void TGACompiler::swap_red_blue()
{
	for (uint64_t i = 0; i < m_image_size * m_image_channels; i += m_image_channels)
	{
		m_image_data[i + 0] ^= m_image_data[i + 2];
		m_image_data[i + 2] ^= m_image_data[i + 0];
		m_image_data[i + 0] ^= m_image_data[i + 2];
	}
}

} // namespace crown

