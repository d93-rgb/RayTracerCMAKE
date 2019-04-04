#pragma once
#include "core/rt.h"
#include <random>

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

	virtual float getSample(int x, int y, float fov) = 0;
	virtual const glm::vec2* get2DArray(int n) = 0;

	const unsigned int samplesPerPixel;
protected:
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
		int current_x, current_y = 0;
		for (int i = 0; i < x * y; ++i)
		{
			for (int j = 0; j < grid_dim; ++j)
			{
				for (int k = 0; k < grid_dim; ++k)
				{
					float u_rnd = float(dist(eng));
					float v_rnd = float(dist(eng));

					sampler2Darray[i].push_back(
						glm::vec2((current_x + (j + u_rnd) / grid_dim),
						(current_y + (k + u_rnd) / grid_dim)));
				}
			}
		}
	}

	const glm::vec2 * get2DArray(int n);

private:
	int grid_dim;
	std::random_device rd;
	std::default_random_engine eng;
	std::uniform_real_distribution<> dist;
};