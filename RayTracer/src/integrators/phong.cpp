#include "integrators/phong.h"

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

glm::vec3 PhongIntegrator::Li()
{
	return glm::vec3();
}


} // namespace rt
