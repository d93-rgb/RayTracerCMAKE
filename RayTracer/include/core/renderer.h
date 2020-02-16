#pragma once
#include "core/rt.h"
#include "interaction/interaction.h"


namespace rt
{
class Renderer
{
public:
	Renderer(size_t w, size_t h, size_t max_depth = 4);


	std::vector<glm::vec3> render_with_threads(unsigned int& width, unsigned int& height);

	// for creating color gradients
	std::vector<glm::vec3> render_gradient(size_t& width_img, const size_t& width_stripe,
		size_t& height);

	std::vector<glm::vec3> render(unsigned int& width, unsigned int& height);

	void run(std::vector<glm::vec3>* colors, std::string& file);


private:
	size_t MAX_DEPTH;
	size_t SPP = 1;
	size_t GRID_DIM = 3;
	size_t NUM_THREADS = 4;

	std::unique_ptr<Image> img;


	float fresnel(float rel_eta, float c);
	
	bool refract(glm::vec3 V, glm::vec3 N, float refr_idx, glm::vec3* refracted);
	
	glm::vec3 reflect(glm::vec3 dir, glm::vec3 N);
	
	float shoot_ray(const Scene& s, const Ray& ray, SurfaceInteraction* isect);
	
	glm::vec3 shoot_recursively(const Scene& s,
		const Ray& ray,
		SurfaceInteraction* isect,
		int depth);
	
	glm::vec3 handle_transmission(const Scene& s,
		const Ray& ray,
		const glm::vec3& isect_p,
		SurfaceInteraction* isect,
		int depth);
	
	glm::vec3 handle_reflection(const Scene& s,
		const Ray& ray,
		const glm::vec3& isect_p,
		SurfaceInteraction* isect,
		int depth);
};

} // namespace rt