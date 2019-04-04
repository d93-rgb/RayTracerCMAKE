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
	Sampler2D(unsigned int x,
		unsigned int y,
		unsigned int spp) : samplesPerPixel(spp)
	{
		for (int i = 0; i < x * y; ++i)
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
	StratifiedSampler2D(unsigned int width,
		unsigned int height,
		unsigned int grid_dim) :
		Sampler2D(width, height, grid_dim* grid_dim),
		grid_dim(grid_dim), eng(rd()), dist(0, 1)
	{
		for (unsigned int i = 0; i < width; ++i)
		{
			for (unsigned int j = 0; j < height; ++j)
			{
				for (int k = 0; k < grid_dim; ++k)
				{
					for (int m = 0; m < grid_dim; ++m)
					{
						float u_rnd = float(dist(eng));
						float v_rnd = float(dist(eng));

						sampler2Darray[i * width + j].push_back(
							glm::vec2((i + (k + u_rnd) / grid_dim),
							(j + (m + u_rnd) / grid_dim)));
					}
				}
			}
		}
	}

	const glm::vec2 * get2DArray();

private:
	int grid_dim;
	std::random_device rd;
	std::default_random_engine eng;
	std::uniform_real_distribution<> dist;
};

} // namespace rt