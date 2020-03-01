#pragma once
#include "core/rt.h"
#include "integrators/integrator.h"

namespace rt
{

class PhongIntegrator : public Integrator
{
public:

	glm::dvec3 Li(const Ray& ray, const Scene& scene, int depth);

private:
	glm::dvec3 diff_shade(
		const Light& light,
		const SurfaceInteraction& isect,
		const glm::dvec3& ob_pos);

	glm::dvec3 spec_shade(
		const Light& light,
		const SurfaceInteraction& isect,
		const glm::dvec3& ob_pos,
		const glm::dvec3& view_dir);

	glm::dvec3 phong_shade(
		const Light& light,
		const Scene& sc,
		const Ray& ray,
		const glm::dvec3& ob_pos,
		const SurfaceInteraction& si);
};

} // namespace rt