#pragma once
#include "core/rt.h"

namespace rt
{
/*
	Sampler for points on a 2D plane.
*/
class Sampler2D
{
public:
	Sampler2D(size_t x,
		size_t y,
		unsigned int spp) : samplesPerPixel(spp)
	{
		for (unsigned int i = 0; i < x * y; ++i)
		{
			sampler2Darray.push_back(std::vector<glm::vec2>(spp));
		}
	}

	~Sampler2D() = default;

	virtual const glm::vec2* get2DArray() = 0;

	const unsigned int samplesPerPixel;
protected:
	unsigned int currentPixel = 0;
	std::vector<std::vector<glm::vec2>> sampler2Darray;
};

class StratifiedSampler2D : public Sampler2D
{
public:
	StratifiedSampler2D(size_t width,
		size_t height,
		size_t grid_dim) :
		Sampler2D(width, height, grid_dim * grid_dim),
		grid_dim(grid_dim)
	{
		std::random_device rd;
		std::default_random_engine eng(rd());
		std::uniform_real_distribution<> dist(0,1);

		for (unsigned int i = 0; i < height; ++i)
		{
			for (unsigned int j = 0; j < width; ++j)
			{
				for (int k = 0; k < grid_dim; ++k)
				{
					for (int m = 0; m < grid_dim; ++m)
					{
						float u_rnd = float(dist(eng));
						float v_rnd = float(dist(eng));

						sampler2Darray[i * width + j][k * grid_dim + m] =
							glm::vec2((k + u_rnd) / grid_dim,
							(m + v_rnd) / grid_dim);
					}
				}
			}
		}
	}

	const glm::vec2 * get2DArray();

private:
	int grid_dim;

};

} // namespace rt