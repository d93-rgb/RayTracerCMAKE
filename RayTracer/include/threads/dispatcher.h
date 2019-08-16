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

	int w_step;
	int h_step;

	unsigned int img_width;
	unsigned int img_height;

	std::vector<std::pair<int, int>> pairs;

	Slice(rt::Image img, int w, int h)
	{
		if ((img.get_height() % 16 != 0) && (img.get_width() % 16 != 0))
		{
			printf("Error : Only image dimensions divisible by 16 supported");
			exit(1);
		}
		w_step = w;
		h_step = h;
		img_width = img.get_width();
		img_height = img.get_height();

		dx = img_width / w;
		dy = img_height / h;

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

void work(Slice& s,
	std::mutex& pairs_mutex,
	std::vector<glm::vec3>& col,
	const Scene& sc,
	StratifiedSampler2D& sampler,
	pbrt::ProgressReporter& reporter,
	unsigned int array_size,
	const glm::vec2* samplingArray,
	float inv_grid_dim,
	float inv_spp,
	float fov_tan,
	float d,
	void (*func)(std::vector<glm::vec3>&,
		const Scene&,
		StratifiedSampler2D&,
		unsigned int,
		const glm::vec2*,
		float,
		float,
		float,
		float,
		int,
		int,
		int,
		int));

//void work(Slice& s,
//	std::mutex& pairs_mutex,
//	std::vector<glm::vec3>& col,
//	const Scene& sc,
//	StratifiedSampler2D& sampler,
//	unsigned int array_size,
//	const glm::vec2* samplingArray,
//	float inv_grid_dim,
//	float inv_spp,
//	float fov_tan,
//	float d) {}
//}

}