#include "integrators/phong.h"
#include "scene/scene.h"
#include "light/light.h"
#include "interaction/interaction.h"
#include "material/material.h"

namespace rt
{

/*
	DEPRECATED
*/
glm::dvec3 PhongIntegrator::diff_shade(
	const Light& light,
	const SurfaceInteraction& isect,
	const glm::dvec3& ob_pos)
{
	glm::dvec3 dir = ob_pos - light.p;
	glm::dvec3 diffuse;
	//double sq_dist = glm::dot(dir, dir); // attenuation

	diffuse = isect.mat->getDiffuse(isect.p);

	glm::dvec3 col = diffuse *
		glm::max(0.0,
			glm::dot(isect.normal,
				-glm::normalize(dir)));

	return col; // sq_dist;
}

/*
	DEPRECATED
	Calculate specular shading of an object.
*/
glm::dvec3 PhongIntegrator::spec_shade(
	const Light& light,
	const SurfaceInteraction& isect,
	const glm::dvec3& ob_pos,
	const glm::dvec3& view_dir)
{
	//glm::dvec3 dir = ob_pos - this->p;
	glm::dvec3 dir = light.p - ob_pos;
	//glm::dvec3 refl = reflect(dir, obj.get_normal(ob_pos));
	glm::dvec3 half = (glm::normalize(dir) - view_dir);
	half /= glm::length(half);
	//refl = glm::normalize(refl);

	return
		isect.mat->getSpecular() *
		pow(glm::max(0.0, glm::dot(half, isect.normal)),
			isect.mat->getShininess());
}

glm::dvec3 PhongIntegrator::phong_shade(
	const Light& light,
	const Scene& sc,
	const Ray& ray,
	const glm::dvec3& ob_pos,
	const SurfaceInteraction& si)
{
	bool visible = true;
	glm::dvec3 color(0);

	if (!light.visible(ob_pos, sc))
	{
		return glm::dvec3(0);
	}

	glm::dvec3 light_dir = light.p - ob_pos;
	glm::dvec3 half = (glm::normalize(light_dir) - ray.rd);
	half /= glm::length(half);

	// ambient
	//color += 0.01 * si.mat->getAmbient(si.p) * light.getEmission(ray.rd);

	// diffuse (lambert)
	color += si.mat->getDiffuse(si.p) * inv_pi;

	// specular
	color += si.mat->getSpecular() *
		pow(glm::max(0.0, glm::dot(half, si.normal)),
			si.mat->getShininess()) /
		glm::abs(glm::dot(si.normal, light_dir));

	return color;
}

RGB_Color PhongIntegrator::Li(const Ray& ray, const Scene& scene, int depth)
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

		Lo += phong_shade(
			*l.get(),
			scene,
			ray,
			isect_p,
			si) * Li * std::abs(glm::dot(si.normal, light_dir)) / pdf;
	}
	// LTE
	// L += f * L * cos(N, L) / pdf

	// TODO: remove from phong integrator, since this belongs to Whitted style ray tracing
	// TODO: change update location or depth may not reach its intended value
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
