#pragma once
#include "rt.h"

namespace rt
{

class PhongIntegrator : public Integrator
{
public:

	virtual glm::vec3 Li();

private:
	virtual glm::dvec3 diff_shade(const SurfaceInteraction& isect,
		const glm::dvec3& ob_pos) = 0;

	virtual glm::dvec3 spec_shade(const SurfaceInteraction& isect,
		const glm::dvec3& ob_pos,
		const glm::dvec3& view_dir) = 0;

	virtual glm::dvec3 phong_shade(const Scene& sc,
		const Ray& ray,
		const glm::dvec3& ob_pos,
		const SurfaceInteraction& isect) = 0;
};

} // namespace rt