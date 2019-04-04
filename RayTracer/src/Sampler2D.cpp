#include "sampler2D.h"

namespace rt
{

const glm::vec2* StratifiedSampler2D::get2DArray()
{
	if (currentPixel == sampler2Darray.size())
	{
		return nullptr;
	}
	return &sampler2Darray[currentPixel++][0];
}

} // namespace rt