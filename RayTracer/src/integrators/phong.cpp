#include "integrators/phong.h"
#include "scene/scene.h"
#include "light/light.h"
#include "interaction/interaction.h"
#include "material/material.h"

namespace rt
{

glm::dvec3 PhongIntegrator::diff_shade(
	const Light& light,
	const SurfaceInteraction& isect, 
	const glm::dvec3& ob_pos)
{
	glm::dvec3 dir = ob_pos - light.p;
	glm::dvec3 diffuse;
	//double sq_dist = glm::dot(dir, dir); // attenuation

	diffuse = isect.mat->getDiffuse(isect.p);

	glm::dvec3 col = light.getEmission(dir) * diffuse *
		glm::max(0.0,
			glm::dot(isect.normal,
				-glm::normalize(dir)));

	return col; // sq_dist;
}

/*
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

	return light.getEmission(view_dir) *
		isect.mat->getSpecular() *
		pow(glm::max(0.0, glm::dot(half, isect.normal)),
			isect.mat->getShininess());
}

glm::dvec3 PhongIntegrator::phong_shade(
	const Light& light,
	const Scene& sc,
	const Ray& ray,
	const glm::dvec3& ob_pos,
	const SurfaceInteraction& isect)
{
	bool visible = true;
	glm::dvec3 color(0);

	glm::dvec3 dir = light.p - ob_pos;
	double sqd_dist = glm::dot(dir, dir);

	//if (sqd_dist > 1.f) sqd_dist *= 0.1f;

	visible = light.visible(ob_pos, sc);

	color = 0.01 * isect.mat->getAmbient(isect.p) * light.getEmission(ray.rd);

	if (visible) {
		color += (diff_shade(light, isect, ob_pos) +
			spec_shade(light, isect, ob_pos, ray.rd)) / sqd_dist;
	}
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
	RGB_Color contribution = glm::dvec3(0);

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
	for (auto& l : scene.lights)
	{
		contribution += phong_shade(
			*l.get(), 
			scene,
			ray/*Ray(ray.ro + shadowEpsilon * ray.rd, ray.rd)*/,
			isect_p,
			si);
	}
	// LTE
	// L += f * L * cos(N, L) / pdf

	//TODO change update location or depth may not reach its intended value
	++depth;
	if (glm::length(si.mat->getReflective()) > 0)
	{
		glm::dvec3 reflective = si.mat->getReflective();
		contribution += reflective * specular_reflect(scene, ray, isect_p, &si, depth);
	}

	if (glm::length(si.mat->getTransparent()) > 0)
	{
		glm::dvec3 transparent = si.mat->getTransparent();
		contribution += transparent * specular_transmit(scene, ray, isect_p, &si, depth);
	}

	return contribution;
}


} // namespace rt
