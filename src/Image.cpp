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

#include "Image.h"
#include "Log.h"
#include "Types.h"
#include "Stream.h"

namespace crown
{

Image::Image() :
	mPixelFormat(PF_UNKNOWN),
	mWidth(0),
	mHeight(0),
	mBuffer(NULL)
{
}

Image::Image(PixelFormat pixelFormat, uint32_t width, uint32_t height, uint8_t* data) :
	mPixelFormat(pixelFormat),
	mWidth(width),
	mHeight(height),
	mBuffer(data)
{
	if (data == NULL)
	{
		CreateBuffer();
		SetUniformColorImage(Color4::WHITE);
	}
}

Image::~Image()
{
	DestroyImage();
}

void Image::CreateImage(PixelFormat pixelFormat, uint32_t width, uint32_t height, uint8_t* data)
{
	mPixelFormat = pixelFormat;
	mWidth = width;
	mHeight = height;
	mBuffer = data;
}

void Image::DestroyImage()
{
	if (mBuffer)
	{
		delete[] mBuffer;
	}

	mPixelFormat = PF_UNKNOWN;
	mWidth = 0;
	mHeight = 0;
	mBuffer = NULL;
}

void Image::SetUniformColorImage(Color4 color)
{
	AssertRGB8();
	int32_t bpp = GetBytesPerPixel();

	uint8_t red   = (uint8_t)(color.r * 255.0f);
	uint8_t green = (uint8_t)(color.g * 255.0f);
	uint8_t blue  = (uint8_t)(color.b * 255.0f);

	for(uint32_t i = 0; i < mHeight; i++)
	{
		int32_t rowOffset = i * mWidth * bpp;
		for(uint32_t j = 0; j < mWidth; j++)
		{
			int32_t offset = rowOffset + bpp * j;
			mBuffer[offset    ] = red;
			mBuffer[offset + 1] = green;
			mBuffer[offset + 2] = blue;
		}
	}
}

uint32_t Image::GetWidth() const
{
	return mWidth;
}

uint32_t Image::GetHeight() const
{
	return mHeight;
}

PixelFormat Image::GetFormat() const
{
	return mPixelFormat;
}

uint32_t Image::GetBitsPerPixel() const
{
	return Pixel::GetBitsPerPixel(mPixelFormat);
}

uint32_t Image::GetBytesPerPixel() const
{
	return Pixel::GetBytesPerPixel(mPixelFormat);
}

uint8_t* Image::GetBuffer()
{
	return mBuffer;
}

const uint8_t* Image::GetBuffer() const
{
	return mBuffer;
}

void Image::ApplyColorKeying(const Color4& color)
{
	assert(mPixelFormat == PF_RGBA_8);

	for (uint64_t i = 0; i < mWidth * mHeight * 4; i += 4)
	{
		if (Color4(mBuffer[i], mBuffer[i+1], mBuffer[i+2]) == color)
		{
			mBuffer[i] = 0;
			mBuffer[i+1] = 0;
			mBuffer[i+2] = 0;
			mBuffer[i+3] = 0;
		}
	}
}

void Image::ApplyGreyscaleToAlpha(Image* greyscaleImage)
{
	if (mPixelFormat != PF_RGBA_8)
	{
		Log::e("Image::ApplyGreyscaleToAlpha: Can apply alpha only on RGBA8 pixel formats.");
		return;
	}

	if (greyscaleImage == NULL)
	{
		Log::e("Image::ApplyGreyscaleToAlpha: greyscaleImage is NULL.");
		return;
	}

	if (greyscaleImage->mPixelFormat != PF_RGBA_8)
	{
		Log::e("Image::ApplyGreyscaleToAlpha: greyscaleImage must have pixel format RGBA8.");
		return;
	}

	if (mWidth != greyscaleImage->mWidth || mHeight != greyscaleImage->mHeight)
	{
		Log::e("Image::ApplyGreyscaleToAlpha: greyscaleImage must have the same dimensions of the image.");
		return;
	}

	for (uint64_t i = 0; i < mWidth * mHeight * 4; i += 4)
	{
		mBuffer[i+3] = greyscaleImage->mBuffer[i];
	}
}

void Image::AlphaToGreyscale()
{
	for (uint64_t i = 0; i < mWidth * mHeight * 4; i += 4)
	{
		mBuffer[i]   = mBuffer[i+3];
		mBuffer[i+1] = mBuffer[i+3];
		mBuffer[i+2] = mBuffer[i+3];
		mBuffer[i+3] = 255;
	}
}

void Image::CreateBuffer()
{
	if (mBuffer == NULL)
	{
		mBuffer = new uint8_t[mWidth * mHeight * GetBytesPerPixel()];
	}
}

void Image::AssertRGB8()
{
	assert(mPixelFormat == PF_RGB_8);
}

void Image::SetPixel(uint32_t x, uint32_t y, Color4 color)
{
	/*AssertRGB8();
	if (x >= mWidth || y >= mHeight)
	{
		throw ArgumentException("Coordinates outside the Image");
	}*/

	int32_t bpp = 3;//GetBytesPerPixel();
	int32_t offset = (y * mWidth + x) * GetBytesPerPixel();
	mBuffer[offset    ] = (uint8_t)(color.r * 255);
	mBuffer[offset + 1] = (uint8_t)(color.g * 255);
	mBuffer[offset + 2] = (uint8_t)(color.b * 255);
}

void Image::CopyTo(Image& dest) const
{
	dest.mWidth = mWidth;
	dest.mHeight = mHeight;
	dest.mPixelFormat = mPixelFormat;
	dest.CreateBuffer();

	for (uint64_t i = 0; i < mWidth * mHeight * GetBytesPerPixel(); i++)
	{
		dest.mBuffer[i] = mBuffer[i];
	}
}

void Image::ConvertToRGBA8()
{
	assert(mPixelFormat != PF_UNKNOWN);

	uint8_t* newBuf = new uint8_t[mWidth * mHeight * 4];

	uint32_t j = 0;
	for (uint32_t i = 0; i < mWidth * mHeight * 2; i += 2)
	{
		newBuf[j + 0] = mBuffer[i + 0];
		newBuf[j + 1] = mBuffer[i + 0];
		newBuf[j + 2] = mBuffer[i + 0];
		newBuf[j + 3] = mBuffer[i + 1];

		j += 4;
	}

	delete[] mBuffer;
	mBuffer = newBuf;
	mPixelFormat = PF_RGBA_8;
}

} // namespace crown

