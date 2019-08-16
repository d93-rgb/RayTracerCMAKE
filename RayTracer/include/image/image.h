#pragma once

namespace rt
{
class Image
{
	unsigned int width;
	unsigned int height;

public:
	Image(unsigned int width, unsigned int height) :
		width(width), height(height) {}

	unsigned int get_width()
	{
		return width;
	}

	unsigned int get_height()
	{
		return height;
	}
};

}