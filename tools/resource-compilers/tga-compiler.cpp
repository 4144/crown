#include <stdio.h>
#include "Filesystem.h"
#include "Stream.h"
#include "Path.h"
#include "String.h"
#include "Hash.h"
#include "Resource.h"
#include "ResourceArchive.h"
#include "FileStream.h"
#include "Pixel.h"
#include "TextureResource.h"
#include <cstring>

using namespace crown;

struct TGAHeader
{

	char		id_length;			// 00h  Size of Image ID field
	char		color_map_type;		// 01h  Color map type
	char		image_type;			// 02h  Image type code
	char		c_map_spec[5];		// 03h  Color map origin 05h Color map length 07h Depth of color map entries
	uint16_t	x_offset;			// 08h  X origin of image
	uint16_t	y_offset;			// 0Ah  Y origin of image
	uint16_t	width;				// 0Ch  Width of image
	uint16_t	height;				// 0Eh  Height of image
	char		pixel_depth;     	// 10h  Image pixel size
	char		image_descriptor;	// 11h  Image descriptor byte
};

void load_uncompressed(void* dest, Stream* stream, uint32_t width, uint32_t height, uint32_t channels);
void load_compressed(void* dest, Stream* stream, uint32_t width, uint32_t height, uint32_t channels);
void swap_red_blue(uint8_t* data, uint64_t size, uint32_t channels);

/// TGA compiler for "tga" resource type
/// TODO: Explain supported formats, usage etc.
int main(int argc, char** argv)
{
	if (argc != 4)
	{
		printf("Usage: %s /path/to/resources /path/to/compiled resource.tga\n", argv[0]);
		return -1;
	}

	Filesystem fs_root(argv[1]);
	Filesystem fs_dest(argv[2]);
	
	const char* resource = argv[3];
	
	if (!fs_root.exists(resource))
	{
		printf("Fatal: resource %s does not exists. Aborting.\n", resource);
		return -1;
	}
	
	char resource_basename[256];
	char resource_extension[256];
	
	path::basename(resource, resource_basename, 256);
	path::extension(resource, resource_extension, 256);
	
	printf("Resource basename  : %s\n", resource_basename);
	printf("Resource extension : %s\n", resource_extension);
	
	uint32_t resource_basename_hash = hash::fnv1a_32(resource_basename, string::strlen(resource_basename));
	uint32_t resource_extension_hash = hash::fnv1a_32(resource_extension, string::strlen(resource_extension));
	
	printf("Resource basename  (hash) : %X\n", resource_basename_hash);
	printf("Resource extension (hash) : %X\n", resource_extension_hash);

	FileStream* src_file = (FileStream*)fs_root.open(resource, SOM_READ);
	
	//-------------------------------------------------------------------------
	// Read TGA Header
	//-------------------------------------------------------------------------
	
	// The TGA header used throughout the code
	TGAHeader header;
	memset(&header, 0, sizeof(TGAHeader));
	
	// Read the header
	src_file->read(&header, sizeof(TGAHeader));

	// Skip TGA ID
	src_file->skip(header.id_length);

	// Pixel format currently unknown
	PixelFormat format = PF_UNKNOWN;

	// Compute color channels	
	uint32_t channels = header.pixel_depth / 8;
	
	// Compute image size
	uint64_t image_size = header.width * header.height;
	
	uint8_t* image_data = NULL;

	// Select the appropriate pixel format and allocate resource data based on tga size and channels
	switch (channels)
	{
		case 2:
		case 3:
		{
			format = PF_RGB_8;
			image_data = new uint8_t[(uint32_t)(image_size * 3)];
			
			break;
		}
		case 4:
		{
			format = PF_RGBA_8;
			image_data = new uint8_t[(uint32_t)(image_size * channels)];
			
			break;
		}
		default:
		{
			printf("Fatal: Unable to determine TGA channels. Aborting.\n");
			return -1;
		}
	}
	
	printf("Debug: w = %d, h = %d, channels = %d\n", header.width, header.height, channels);

	// Determine image type (compressed/uncompressed) and call proper function to load TGA
	switch (header.image_type)
	{
		case 0:
		{
			printf("Fatal: The resource does not contain image data. Aborting.");
			return -1;
		}
		case 2:
		{
			printf("Debug: loading uncompressed...\n");
			load_uncompressed(image_data, src_file, header.width, header.height, channels);
			break;
		}

		case 10:
		{
			printf("Debug: loading compressed...\n");
			load_compressed(image_data, src_file, header.width, header.height, channels);
			break;
		}

		default:
		{
			printf("Fatal: Image type not supported. Aborting.");
			return -1;
		}
	}

	// FIXME Fixed options for now until proper settings management implemented
	TextureMode		mode = TM_MODULATE;
	TextureFilter	filter = TF_BILINEAR;
	TextureWrap		wrap = TW_REPEAT;
	
	// Open output file
	FileStream* dest_file = (FileStream*)fs_dest.open(resource, SOM_WRITE);
	
	ArchiveEntry archive_entry;
	archive_entry.name = resource_basename_hash;
	archive_entry.type = resource_extension_hash;
	archive_entry.offset = sizeof(ArchiveEntry);
	archive_entry.size = image_size * channels + sizeof(PixelFormat) + sizeof(uint16_t) * 2 +
							sizeof(TextureMode) + sizeof(TextureFilter) + sizeof(TextureWrap);
							
	// Write out the archive entry
	dest_file->write(&archive_entry, sizeof(ArchiveEntry));

	// Write out the data
	dest_file->write(&format, sizeof(PixelFormat));
	dest_file->write(&header.width, sizeof(uint16_t));
	dest_file->write(&header.height, sizeof(uint16_t));
	
	dest_file->write(&mode, sizeof(TextureMode));
	dest_file->write(&filter, sizeof(TextureFilter));
	dest_file->write(&wrap, sizeof(TextureWrap));
	
	dest_file->write(image_data, image_size * channels);
	
	// Done, free the resources and exit
	if (image_data != NULL)
	{
		delete[] image_data;
	}

	return 0;
}

//-----------------------------------------------------------------------------
void load_uncompressed(void* dest, Stream* stream, uint32_t width, uint32_t height, uint32_t channels)
{
	uint64_t size = width * height;
	
	uint8_t* data = (uint8_t*)dest;

	if (channels == 2)
	{
		int32_t j = 0;

		for (uint64_t i = 0; i < size * channels; i++)
		{
			uint16_t pixel_data;
			
			stream->read(&pixel_data, sizeof(pixel_data));
			
			data[j + 0] = (pixel_data & 0x7c) >> 10;
			data[j + 1] = (pixel_data & 0x3e) >> 5;
			data[j + 2] = (pixel_data & 0x1f);
			
			j += 3;
		}
	}
	else
	{
		stream->read(data, (size_t)(size * channels));

		swap_red_blue(data, size * channels, channels);
	}
}

//-----------------------------------------------------------------------------
void load_compressed(void* dest, Stream* stream, uint32_t width, uint32_t height, uint32_t channels)
{
	uint8_t rle_id = 0;
	uint32_t i = 0;
	uint32_t colors_read = 0;
	uint64_t size = width * height;
	
	uint8_t* data = (uint8_t*)dest;

	uint8_t* colors = new uint8_t[channels];

	while (i < size)
	{
		stream->read(&rle_id, sizeof(uint8_t));

		// If MSB == 1
		if (rle_id & 0x80)
		{
			rle_id -= 127;
			
			stream->read(colors, channels);

			while (rle_id)
			{
				data[colors_read + 0] = colors[2];
				data[colors_read + 1] = colors[1];
				data[colors_read + 2] = colors[0];

				if (channels == 4)
				{
					data[colors_read + 3] = colors[3];
				}

				rle_id--;
				colors_read += channels;
				i++;
			}
		}
		else
		{
			rle_id++;

			while (rle_id)
			{
				stream->read(colors, channels);
				
				data[colors_read + 0] = colors[2];
				data[colors_read + 1] = colors[1];
				data[colors_read + 2] = colors[0];

				if (channels == 4)
				{
					data[colors_read + 3] = colors[3];
				}

				rle_id--;
				colors_read += channels;
				i++;
			}
		}
	}

	delete[] colors;
}

//-----------------------------------------------------------------------------
void swap_red_blue(uint8_t* data, uint64_t size, uint32_t channels)
{
	for (uint64_t i = 0; i < size; i += channels)
	{
		data[i] ^= data[i+2];
		data[i+2] ^= data[i];
		data[i] ^= data[i+2];
	}
}

