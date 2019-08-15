#pragma once
#include "core/rt.h"
#include "image/image.h"

namespace rt
{
class Dispatcher
{
	Dispatcher() = default;

	Slice slice_image(Image img, unsigned int w, unsigned int h);
};

class Slice
{
public:
	Slice(int dx, int dy) :
		dx(dx), dy(dy) {}

	std::vector<std::pair<int, int>> pairs;
private:
	int dx;
	int dy;

};

}
