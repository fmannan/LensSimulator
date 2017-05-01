#include "image.h"
#include <QImage>
#include <cassert>

Image::Image(void) : mWidth(0), mHeight(0), mBuffer(nullptr)
{
}

Image::Image(Image&& im)
{
	mWidth = im.mWidth;
	mHeight = im.mHeight;
	mBuffer = im.mBuffer;
	im.mBuffer = nullptr;
}

Image::Image(int width, int height) : mWidth(width), mHeight(height)
{
	mBuffer = new Colorf[width * height];
}

Image::~Image(void)
{
	if(mBuffer)
		delete [] mBuffer;
	mBuffer = nullptr;
}

void Image::set(int row, int col, float r, float g, float b, float a)
{
	mBuffer[row * mWidth + col].set(r, g, b, a);
}

Colorf Image::get(int row, int col)
{
	return mBuffer[row * mWidth + col];
}

void Image::normalize_max()
{
	Colorf maxC(0.0f);

	for(int idx = 0; idx < mWidth * mHeight; idx++) {
		if(mBuffer[idx] > maxC) {
			maxC = mBuffer[idx];
		}
	}

	for(int idx = 0; idx < mWidth * mHeight; idx++) {
		mBuffer[idx] = mBuffer[idx] / maxC;
	}
}

void Image::save(const std::string& filename, const std::string& format) const
{
//	BYTE *data = new BYTE[3 * mWidth * mHeight];
//	for(int row = 0; row < mHeight; row++) {
//		for(int col = 0; col < mWidth; col++) {
//			const float *rgba = mBuffer[(mHeight - row - 1) * mWidth + col].get();
//			data[3 * (row * mWidth + col)] = static_cast<BYTE>(glm::min(glm::round(255 * rgba[2]), 255.0f));
//			data[3 * (row * mWidth + col) + 1] = static_cast<BYTE>(glm::min(glm::round(255 * rgba[1]), 255.0f));
//			data[3 * (row * mWidth + col) + 2] = static_cast<BYTE>(glm::min(glm::round(255 * rgba[0]), 255.0f));
//		}
//	}
	 
//	FIBITMAP* img = FreeImage_ConvertFromRawBits(data, mWidth, mHeight, mWidth * 3, 24, 0xFF0000, 0x00FF00, 0x0000FF, false);

//	FreeImage_Save(FIF_PNG, img, filename.c_str(), 0);
//	delete [] data;
}
