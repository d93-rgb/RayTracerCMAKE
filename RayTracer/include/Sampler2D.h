#pragma once
#include <random>

/*
	Sampler for points on a 2D plane.
*/
class Sampler2D
{
public:
	Sampler2D(size_t imag_w, size_t imag_h, int grid_w, int grid_h) :
		imag_w(imag_w), imag_h(imag_h), grid_w(grid_w), grid_h(grid_h),
		eng(rd()), dist(0, 1)
	{

	}

	float getUniformSample(int x, int y, float fov);
	~Sampler2D();


private:
	size_t imag_w, imag_h;
	int grid_w, grid_h;
	std::random_device rd;
	std::default_random_engine eng;
	std::uniform_real_distribution<> dist;
};

