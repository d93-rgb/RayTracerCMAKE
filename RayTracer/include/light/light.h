#pragma once
#include "core/rt.h"
#include "interaction/interaction.h"

namespace rt
{
struct Light
{
	glm::dvec3 p;
	glm::dvec3 dir;
	glm::dvec3 emission;
	double power;

	Light(glm::dvec3 p, glm::dvec3 dir, glm::dvec3 col) :
		power(0)
	{
		this->p = p;
		this->dir = glm::normalize(dir);
		this->emission = col;
	}

	~Light();

	virtual glm::dvec3 getEmission(glm::dvec3 dir) const = 0;

	virtual glm::dvec3 diff_shade(const SurfaceInteraction & isect,

		const glm::dvec3 &ob_pos) = 0;

	virtual glm::dvec3 spec_shade(const SurfaceInteraction & isect,
		const glm::dvec3 &ob_pos,
		const glm::dvec3 &view_dir) = 0;

	virtual bool calc_shadow(glm::dvec3 p, const Scene &sc) = 0;

	virtual glm::dvec3 phong_shade(const Scene & sc,
		const Ray & ray,
		const glm::dvec3 & ob_pos, 
		const SurfaceInteraction & isect) = 0;

};


struct PointLight : public Light
{
	PointLight(glm::dvec3 p, glm::dvec3 dir, glm::dvec3 col) :
		Light(p, dir, col)
	{
	}

	// equal light emission in all directions
	glm::dvec3 getEmission(glm::dvec3 dir) const
	{
		return emission;
	}

	glm::dvec3 diff_shade(const SurfaceInteraction & isect, const glm::dvec3 & ob_pos);
	glm::dvec3 spec_shade(const SurfaceInteraction & isect, const glm::dvec3 & ob_pos, const glm::dvec3 & view_dir);
	bool calc_shadow(glm::dvec3 p, const Scene &sc);
	glm::dvec3 phong_shade(const Scene & sc, const Ray & ray, const glm::dvec3 & ob_pos, const SurfaceInteraction & isect);
};

struct PointLightShaped : public Light
{
	PointLightShaped(glm::dvec3 p, glm::dvec3 dir, glm::dvec3 col) :
		Light(p, dir, col)
	{
	}

	// equal light emission in all directions
	glm::dvec3 getEmission(glm::dvec3 dir) const
	{
		return emission;
	}

	glm::dvec3 diff_shade(const SurfaceInteraction & isect, const glm::dvec3 & ob_pos);
	glm::dvec3 spec_shade(const SurfaceInteraction & isect, const glm::dvec3 & ob_pos, const glm::dvec3 & view_dir);
	bool calc_shadow(glm::dvec3 p, const Scene &sc);
	glm::dvec3 phong_shade(const Scene & sc, const Ray & ray, const glm::dvec3 & ob_pos, const SurfaceInteraction & isect);

};

struct SpotLight : public Light
{

};

//struct AreaLight : public Light
//{
//
//};

struct DistantLight : public Light
{
	DistantLight(glm::dvec3 dir, glm::dvec3 col) :
		Light(glm::dvec3(INFINITY), glm::normalize(dir), col)
	{
	}

	glm::dvec3 getEmission(glm::dvec3 dir) const
	{
		return emission;
	}
	glm::dvec3 diff_shade(const SurfaceInteraction & isect, const glm::dvec3 & ob_pos);
	glm::dvec3 spec_shade(const SurfaceInteraction & isect, const glm::dvec3 & ob_pos, const glm::dvec3 & view_dir);
	bool calc_shadow(glm::dvec3 p, const Scene &sc);
	glm::dvec3 phong_shade(const Scene & sc, const Ray & ray, const glm::dvec3 & ob_pos, const SurfaceInteraction & isect);
};
}