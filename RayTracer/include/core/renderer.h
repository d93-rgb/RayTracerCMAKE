#pragma once
#include "core/rt.h"
#include "interaction/interaction.h"


namespace rt
{

enum class RenderMode {
	NO_THREADS, THREADS, GRADIENT
};

class Renderer
{
public:
	Renderer(size_t w, size_t h, const std::string& file, size_t max_depth = 4);

	void render(size_t& width, size_t& height);

	void render_with_threads(size_t& width, size_t& height);

	// for creating color gradients
	void render_gradient(size_t& width_img, const size_t& width_stripe,
		size_t& height);

	void run(RenderMode mode);

	std::vector<glm::dvec3> get_colors() const;

private:
	size_t MAX_DEPTH;

	// samples per pixel
	size_t SPP;
	
	size_t GRID_DIM;
	size_t NUM_THREADS;

	std::vector<glm::dvec3> colors;
	std::unique_ptr<Image> img;


	double fresnel(double rel_eta, double c);
	
	bool refract(glm::dvec3 V, glm::dvec3 N, double refr_idx, glm::dvec3* refracted);
	
	glm::dvec3 reflect(glm::dvec3 dir, glm::dvec3 N);
	
	double shoot_ray(const Scene& s, const Ray& ray, SurfaceInteraction* isect);
	
	glm::dvec3 shoot_recursively(const Scene& s,
		const Ray& ray,
		SurfaceInteraction* isect,
		int depth);
	
	glm::dvec3 handle_transmission(const Scene& s,
		const Ray& ray,
		const glm::dvec3& isect_p,
		SurfaceInteraction* isect,
		int depth);
	
	glm::dvec3 handle_reflection(const Scene& s,
		const Ray& ray,
		const glm::dvec3& isect_p,
		SurfaceInteraction* isect,
		int depth);
};

} // namespace rt