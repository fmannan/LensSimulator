#pragma once
#include <string>
#include "shadinginfo.h"
// TODO: Need to make it more efficient
class Image
{
	int mWidth;
	int mHeight;

	Colorf *mBuffer;
public:
	Image(void);
	Image(Image&& im);
	Image(int width, int height);
	~Image(void);

	Colorf get(int row, int col);
	void set(int row, int col, float r, float g, float b, float a = 0);
    void save(const std::string& filename, const std::string& format = "png") const;
	void normalize_max();
};

