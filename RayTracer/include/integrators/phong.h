#pragma once
#include "core/rt.h"
#include "integrators/integrator.h"

namespace rt
{

class PhongIntegrator : public Integrator
{
public:

	glm::dvec3 PhongIntegrator::Li(const Ray& ray, const Scene& scene, int depth);

private:
	glm::dvec3 diff_shade(const SurfaceInteraction& isect,
		const glm::dvec3& ob_pos) = 0;

	glm::dvec3 spec_shade(const SurfaceInteraction& isect,
		const glm::dvec3& ob_pos,
		const glm::dvec3& view_dir) = 0;

	glm::dvec3 phong_shade(const Scene& sc,
		const Ray& ray,
		const glm::dvec3& ob_pos,
		const SurfaceInteraction& isect) = 0;
};

} // namespace rt