#include "..\..\include\threads\dispatcher.h"

namespace rt
{
	// split an image into rasters of width w and height h
	Slice Dispatcher::slice_image(Image img, unsigned int w, unsigned int h)
	{
		int dx = img.get_width() / w;
		int dy = img.get_height() / h;

		std::vector<std::pair<int, int>> pairs;
		Slice s(dx, dy);

		for (int i = 0; i < dx; ++i)
		{
			for (int j = 0; j < dy; ++j) 
			{
				s.pairs.push_back(std::pair<int, int>(i * dx, j * dy));
			}
		}

		return s;
	}
}