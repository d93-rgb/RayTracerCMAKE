#include "core/rt.h"
#include "light/light.h"
#include "shape/shape.h"
#include "scene/scene.h"
#include "camera/camera.h"

namespace rt
{

Light::~Light() {}


glm::dvec3 PointLight::diff_shade(const SurfaceInteraction & isect, const glm::dvec3 & ob_pos)
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
glm::dvec3 PointLight::spec_shade(const SurfaceInteraction & isect,
	const glm::dvec3 &ob_pos,
	const glm::dvec3 &view_dir)
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

/*
	Return true if object is visible to the light and false otherwise
*/
bool PointLight::calc_shadow(glm::dvec3 p, const Scene &sc)
{
	double dist;
	double t_int = INFINITY;
	double tmp = INFINITY;

	glm::dvec3 dist_v = this->p - p;

	Ray ray = Ray(p, glm::normalize(dist_v));

	dist = glm::length(dist_v);
	ray.ro += ray.rd * shadowEpsilon;

	SurfaceInteraction isect;

	// send shadow rays
	for (auto &objs : sc.get_scene())
	{
		tmp = objs->intersect(ray, &isect);

		if (tmp >= 0 && t_int > tmp)
		{
			t_int = tmp;
		}
	}
	// no intersection found
	if (t_int < 0 || t_int == INFINITY || t_int > dist)
	{
		return true;
	}

	return false;
}


glm::dvec3 PointLight::phong_shade(const Scene &sc,
	const Ray &ray,
	const glm::dvec3 &ob_pos,
	const SurfaceInteraction & isect)
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

glm::dvec3 DistantLight::diff_shade(const SurfaceInteraction & isect,

	const glm::dvec3 &ob_pos)
{
	double angle = glm::max(0.0,
		glm::dot(isect.normal, -this->dir));

	if (angle <= 0)
	{
		return glm::dvec3(0.f);
	}

	return getEmission(this->p - ob_pos) * isect.mat->getDiffuse(isect.p) *
		angle;
}


glm::dvec3 DistantLight::spec_shade(const SurfaceInteraction & isect,

	const glm::dvec3 &ob_pos,
	const glm::dvec3 &view_dir)
{
	glm::dvec3 refl = reflect(this->dir, isect.normal);
	double angle = glm::max(0.0,
		glm::dot(isect.normal, -this->dir));

	if (angle <= 0)
	{
		return glm::dvec3(0.0);
	}

	refl = glm::normalize(refl);

	return getEmission(view_dir) * isect.mat->getSpecular() *
		pow(angle, isect.mat->getShininess());
}

bool DistantLight::calc_shadow(glm::dvec3 p, const Scene &sc)
{
	double t_int = INFINITY;
	double tmp = INFINITY;

	Ray ray = Ray(p, -this->dir);
	ray.ro += ray.rd * shadowEpsilon;

	SurfaceInteraction isect;

	// send shadow rays
	for (auto &objs : sc.get_scene())
	{
		tmp = objs->intersect(ray, &isect);

		if (tmp >= 0 && t_int > tmp)
		{
			t_int = tmp;
		}
	}
	// no intersection found
	if (t_int < 0 || t_int == INFINITY) return true;

	return false;
}


glm::dvec3 DistantLight::phong_shade(const Scene &sc,
	const Ray &ray,
	const glm::dvec3 &ob_pos,
	const SurfaceInteraction &isect)
{
	bool visible = true;
	glm::dvec3 color(0);

	visible = calc_shadow(ob_pos, sc);
	color = isect.mat->getAmbient(isect.p) * getEmission(ray.rd);

	if (visible) {
		color += diff_shade(isect, ob_pos) +
			spec_shade(isect, ob_pos, ray.rd);

	}
	return color;
}
}
