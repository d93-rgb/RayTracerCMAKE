#pragma once
#include <thread>
#include <mutex>
#include "core/rt.h"
#include "image/image.h"

namespace rt
{
class Dispatcher
{
	Dispatcher() = default;

};

class Slice
{
public:
	// deprecated! change name to sth that describes 'amount of steps'
	// mroe properly
	int dx;
	int dy;

	size_t w_step;
	size_t h_step;

	size_t img_width;
	size_t img_height;

	std::vector<std::pair<int, int>> pairs;

	Slice(const rt::Image& img, int w, int h)
	{
		/*if ((img.get_height() % 16 != 0) || (img.get_width() % 16 != 0))
		{
			printf("ERROR: Only image dimensions divisible by 16 supported\n");
			exit(1);
		}*/
		w_step = w;
		h_step = h;
		img_width = img.get_width();
		img_height = img.get_height();

		/*dx = int(std::ceil(img_width / double(w)));
		dy = int(std::ceil(img_height / double(h)));
*/
		dx = static_cast<int>(img_width / w + (img_width % w == 0 ? 0 : 1));
		dy = static_cast<int>(img_height / h + (img_height % h == 0 ? 0 : 1));

		idx = -1;

		for (int i = 0; i < dx; ++i)
		{
			for (int j = 0; j < dy; ++j)
			{
				pairs.push_back(std::pair<int, int>(i * w, j * h));
			}
		}
		length = pairs.size();
	}

	/*
		Return next index of the raster
		If no index is left, -1 is returned
	*/
	int get_index();

	size_t get_length()
	{
		return length;
	}

private:
	// index variable for the vector of coordinate pairs
	int idx;
	// size of 'pairs'
	size_t length;
};

}