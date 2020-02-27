#include "shape/ray.h"
#include "interaction/interaction.h"
#include "integrators/integrator.h"
#include "scene/scene.h"

namespace rt
{
/*
	Calculate the normalized reflection vector.
	dir	: the incident ray
	N	: the normalized normal vector of a surface
*/
glm::dvec3 Integrator::reflect(glm::dvec3 dir, glm::dvec3 N)
{
	return glm::normalize(dir - 2 * glm::dot(N, dir) * N);
}

/*
	Calculate the normalized refracted vector.
	V	: the view direction
	N	: the normalized normal vector of a surface
*/
bool Integrator::refract(glm::dvec3 V, glm::dvec3 N, double refr_idx, glm::dvec3* refracted)
{
	double cos_alpha = glm::dot(-V, N);

	// TODO: calculate refracted ray when coming from a medium other than air
	// refractive index of air = 1.f
	double eta = 1.0 / refr_idx;

	if (cos_alpha < 0.f)
	{
		eta = 1.0 / eta;
		cos_alpha *= -1;
		N = -N;
	}

	double radicand = 1.0 - eta * eta * (1.0 - cos_alpha * cos_alpha);

	// check for total internal reflection
	if (radicand < 0.0)
	{
		*refracted = glm::dvec3(0.0);
		return false;
	}

	*refracted = glm::normalize(eta * V - (eta * cos_alpha + sqrt(radicand)) * N);
	return true;
}

/*
	Compute the fresnel term, that is, the factor for reflective contribution
	rel_eta: the relative refractive coefficient
	c: the cosine of the angle between incident and normal ray
*/
double Integrator::fresnel(double rel_eta, double c)
{

	if (c < 0.0)
	{
		c = -c;
		rel_eta = 1.0 / rel_eta;
	}
	// using Schlick's approximation
	double r0 = (rel_eta - 1.0) / (rel_eta + 1.0);
	r0 = r0 * r0;

	c = 1.0 - c;

	return r0 + (1.0 - r0) * pow(c, 5);
}

glm::dvec3 Integrator::specular_reflect(const Scene& s,
	const Ray& ray,
	const glm::dvec3& isect_p,
	SurfaceInteraction* isect,
	int depth)
{
	glm::dvec3 reflected = reflect(ray.rd, isect->normal);

	auto new_ray = Ray(isect_p + shadowEpsilon * reflected, reflected);

	return Li(
		Ray(isect_p + shadowEpsilon * reflected, reflected),
		s,
		depth);
}

glm::dvec3 Integrator::specular_transmit(const Scene& s,
	const Ray& ray,
	const glm::dvec3& isect_p,
	SurfaceInteraction* isect,
	int depth)
{
	glm::dvec3 reflected, refracted;
	double f;

	reflected = reflect(ray.rd, isect->normal);

	// check for total internal reflection
	if (!refract(ray.rd, isect->normal, isect->mat->getRefractiveIdx(), &refracted))
	{
		//reflected = glm::normalize(reflect(ray.rd, (*o)->get_normal(isect_p)));
		return Li(
			Ray(isect_p + shadowEpsilon * reflected, reflected),
			s,
			depth);
	}

	f = fresnel(1.f / isect->mat->getRefractiveIdx(),
		glm::dot(-ray.rd, isect->normal));
	depth;

	return f * Li(
		Ray(isect_p + shadowEpsilon * reflected, reflected),
		s,
		depth) +
		(1.f - f) * Li(
			Ray(isect_p + shadowEpsilon * refracted, refracted),
			s,
			depth);
}


} // namespace