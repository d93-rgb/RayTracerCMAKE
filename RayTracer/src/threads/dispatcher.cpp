#include "..\..\include\threads\dispatcher.h"

namespace rt
{

int Slice::get_index()
{
	if (idx == length || ++idx == length)
	{
		return -1;
	}
	else
	{
		return idx;
	}
}

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
	void (*func)(std::vector<glm::vec3>& col,
		const Scene& sc,
		StratifiedSampler2D& sampler,
		unsigned int array_size,
		const glm::vec2* samplingArray,
		float inv_grid_dim,
		float inv_spp,
		float fov_tan,
		float d,
		int x,
		int y,
		int x1,
		int y1))
{
	int idx = 0;

	while (idx != -1)
	{
		// try to access the next free raster
		pairs_mutex.lock();
		idx = s.get_index();
		pairs_mutex.unlock();

		if (idx < 0)
		{
			break;
		}

		assert(idx < s.get_length());

		for (int i = 0; i < s.h_step; ++i)
		{
			for (int j = 0; j < s.w_step; ++j)
			{
				func(col, sc, sampler, array_size, samplingArray, inv_grid_dim, inv_spp,
					fov_tan, d, s.pairs[idx].first + i, s.pairs[idx].second + j, (s.pairs[idx].second + i) * s.img_width, s.pairs[idx].first + j);
			}
		}	

		reporter.Update();
	}
}

}