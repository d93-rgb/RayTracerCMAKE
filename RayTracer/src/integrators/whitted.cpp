#include "integrators/whitted.h"
#include "scene/scene.h"
#include "interaction/interaction.h"
#include "light/light.h"
#include "material/bsdf.h"


namespace rt
{

RGB_Color WhittedIntegrator::Li(const Ray& ray, const Scene& scene, int depth)
{
	if (depth == scene.MAX_DEPTH)
	{
		return glm::dvec3(0);
	}

	glm::dvec3 isect_p;
	SurfaceInteraction si;
	double distance;
	RGB_Color Lo = glm::dvec3(0); // received radiance at camera point

	// map direction of normals to a color for debugging
#ifdef DEBUG_NORMALS
	return contribution = (glm::dvec3(1.f) + isect->normal) * 0.5f;
#endif

	distance = scene.shoot_ray(ray, &si);

	// check for no intersection
	if (distance < 0 || distance == INFINITY)
	{
		return glm::dvec3(0.0f);
	}

	isect_p = ray.ro + distance * ray.rd;

	// TODO: handle shadows correctly
	// TODO: enhance support for different light types
	// TODO: Add emission term Le if area light source was hit
	// => Lo += Le;

	for (auto& l : scene.lights)
	{
		glm::dvec3 light_dir;
		double pdf;

		RGB_Color Li = l->sample_light(isect_p, light_dir, pdf);

		if (Li == glm::dvec3(0.0) || pdf == 0.0)
		{
			continue;
		}

		RGB_Color f = si.bsdf->f(ray.rd, light_dir);
		Lo += f * Li * std::abs(glm::dot(si.normal, light_dir)) / pdf;
	}

	//TODO change update location or depth may not reach its intended value
	++depth;
	if (glm::length(si.mat->getReflective()) > 0)
	{
		glm::dvec3 reflective = si.mat->getReflective();
		Lo += reflective * specular_reflect(scene, ray, isect_p, &si, depth);
	}

	if (glm::length(si.mat->getTransparent()) > 0)
	{
		glm::dvec3 transparent = si.mat->getTransparent();
		Lo += transparent * specular_transmit(scene, ray, isect_p, &si, depth);
	}

	return Lo;
}

} // namespace rt
