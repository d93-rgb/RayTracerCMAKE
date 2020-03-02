#pragma once
#include "core/rt.h"
#include "interaction/interaction.h"


namespace rt
{

enum class RenderMode {
	THREADS, GRADIENT, ANIMATE
};

class Renderer
{
public:
	Renderer(size_t w, size_t h, const std::string& file, size_t max_depth = 4);

	void render_with_threads(size_t& width, size_t& height);

	void render_with_threads(
		size_t& width,
		size_t& height,
		double degree);

	// for creating color gradients
	void render_gradient(size_t& width_img, const size_t& width_stripe,
		size_t& height);

	void run(RenderMode mode);

	std::vector<glm::dvec3> get_colors() const;

	glm::u64vec2 get_image_dim() const;

private:
	size_t MAX_DEPTH;

	// samples per pixel
	size_t SPP;
	
	size_t GRID_DIM;
	size_t NUM_THREADS;

	std::unique_ptr<Image> img;
};

} // namespace rt