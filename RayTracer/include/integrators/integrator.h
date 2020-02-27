#pragma once
#include "core/rt.h"

namespace rt
{

class Integrator
{
public:

	virtual glm::dvec3 Li(const Ray& ray, const Scene& scene, int depth) = 0;

protected:
	bool refract(glm::dvec3 V, glm::dvec3 N, double refr_idx, glm::dvec3* refracted);

	glm::dvec3 reflect(glm::dvec3 dir, glm::dvec3 N);

	double fresnel(double rel_eta, double c);

	glm::dvec3 specular_transmit(const Scene& s,
		const Ray& ray,
		const glm::dvec3& isect_p,
		SurfaceInteraction* isect,
		int depth);

	glm::dvec3 specular_reflect(const Scene& s,
		const Ray& ray,
		const glm::dvec3& isect_p,
		SurfaceInteraction* isect,
		int depth);
};

} // namespace rt