#include "integrators/phong.h"
#include "scene/scene.h"
#include "interaction/interaction.h"
#include "material/material.h"

namespace rt
{

glm::dvec3 PhongIntegrator::diff_shade(const SurfaceInteraction& isect, const glm::dvec3& ob_pos)
{
	glm::dvec3 dir = ob_pos - this->p;
	glm::dvec3 diffuse;
	//double sq_dist = glm::dot(dir, dir); // attenuation

	diffuse = isect.mat->getDiffuse(isect.p);

	glm::dvec3 col = getEmission(dir) * diffuse *
		glm::max(0.0,
			glm::dot(isect.normal,
				-glm::normalize(dir)));

	return col; // sq_dist;
}

/*
	Calculate specular shading of an object.
*/
glm::dvec3 PhongIntegrator::spec_shade(const SurfaceInteraction& isect,
	const glm::dvec3& ob_pos,
	const glm::dvec3& view_dir)
{
	//glm::dvec3 dir = ob_pos - this->p;
	glm::dvec3 dir = this->p - ob_pos;
	//glm::dvec3 refl = reflect(dir, obj.get_normal(ob_pos));
	glm::dvec3 half = (glm::normalize(dir) - view_dir);
	half /= glm::length(half);
	//refl = glm::normalize(refl);

	return getEmission(view_dir) *
		isect.mat->getSpecular() *
		pow(glm::max(0.0, glm::dot(half, isect.normal)),
			isect.mat->getShininess());
}

glm::dvec3 PhongIntegrator::phong_shade(const Scene& sc,
	const Ray& ray,
	const glm::dvec3& ob_pos,
	const SurfaceInteraction& isect)
{
	bool visible = true;
	glm::dvec3 color(0);

	glm::dvec3 dir = p - ob_pos;
	double sqd_dist = glm::dot(dir, dir);

	//if (sqd_dist > 1.f) sqd_dist *= 0.1f;

	visible = calc_shadow(ob_pos, sc);

	color = 0.01 * isect.mat->getAmbient(isect.p) * getEmission(ray.rd);

	if (visible) {
		color += (diff_shade(isect, ob_pos) +
			spec_shade(isect, ob_pos, ray.rd)) / sqd_dist;
	}
	return color;
}

glm::vec3 PhongIntegrator::Li(const Ray& ray, const Scene& scene)
{
	if (depth == scene.MAX_DEPTH)
	{
		return glm::dvec3(0);
	}

	glm::dvec3 isect_p;
	SurfaceInteraction si;
	double distance;
	glm::dvec3 contribution = glm::dvec3(0);

	// map direction of normals to a color for debugging
#ifdef DEBUG_NORMALS
	return contribution = (glm::dvec3(1.f) + isect->normal) * 0.5f;
#endif

	distance = scene.shoot_ray(ray, isect);

	// check for no intersection
	if (distance < 0 || distance == INFINITY)
	{
		return glm::dvec3(0.0f);
	}

	isect_p = ray.ro + distance * ray.rd;

	for (auto& l : scene.lights)
	{
		contribution += phong_shade(scene,
			ray/*Ray(ray.ro + shadowEpsilon * ray.rd, ray.rd)*/,
			si.isect_p,
			*si);
	}

	if (glm::length(si->mat->getReflective()) > 0)
	{
		glm::dvec3 reflective = si->mat->getReflective();
		contribution += reflective * specular_reflect(ray, isect_p, si, depth);
	}

	if (glm::length(isect->mat->getTransparent()) > 0)
	{
		glm::dvec3 transparent = si->mat->getTransparent();
		contribution += transparent * specular_transmit(ray, isect_p, si, depth);
	}

	return glm::vec3();
}


} // namespace rt
