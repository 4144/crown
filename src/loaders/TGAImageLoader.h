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

#include "ImageLoader.h"
#include "Types.h"

namespace crown
{

class Image;
class Stream;

class TGAImageLoader : public ImageLoader
{

	struct TGAHeader_t
	{

		char id_length;        /* 00h  Size of Image ID field */
		char color_map_type;   /* 01h  Color map type */
		char image_type;       /* 02h  Image type code */
		char c_map_spec[5];    /* 03h  Color map origin 05h Color map length 07h Depth of color map entries */
		uint16_t x_offset;        /* 08h  X origin of image */
		uint16_t y_offset;        /* 0Ah  Y origin of image */
		uint16_t width;           /* 0Ch  Width of image */
		uint16_t height;          /* 0Eh  Height of image */
		char pixel_depth;      /* 10h  Image pixel size */
		char image_descriptor; /* 11h  Image descriptor byte */
	};

public:

	TGAImageLoader();
	~TGAImageLoader();

	Image* LoadFile(const char* relativePath);
	void SaveFile(const Image* image, const char* relativePath);

private:

	Image* LoadUncompressedData(Stream* file);
	Image* LoadCompressedData(Stream* file);
	void SwapRedBlue(uint8_t* data, uint64_t size, uint32_t channels);

	TGAHeader_t mTGAHeader;
};

} // namespace crown

