#pragma once
#include "core/rt.h"
#include "interaction/interaction.h"

namespace rt
{
struct Light
{
	glm::vec3 p;
	glm::vec3 dir;
	glm::vec3 emission;
	float power;

	Light(glm::vec3 p, glm::vec3 dir, glm::vec3 col)
	{
		this->p = p;
		this->dir = glm::normalize(dir);
		this->emission = col;
	}

	~Light();

	virtual glm::vec3 getEmission(glm::vec3 dir) const = 0;

	virtual glm::vec3 diff_shade(const SurfaceInteraction & isect,

		const glm::vec3 &ob_pos) = 0;

	virtual glm::vec3 spec_shade(const SurfaceInteraction & isect,
		const glm::vec3 &ob_pos,
		const glm::vec3 &view_dir) = 0;

	virtual bool calc_shadow(glm::vec3 p, const Scene &sc) = 0;

	virtual glm::vec3 phong_shade(const Scene & sc,
		const Ray & ray,
		const glm::vec3 & ob_pos, 
		const SurfaceInteraction & isect) = 0;

};


struct PointLight : public Light
{
	PointLight(glm::vec3 p, glm::vec3 dir, glm::vec3 col) :
		Light(p, dir, col)
	{
	}

	// equal light emission in all directions
	glm::vec3 getEmission(glm::vec3 dir) const
	{
		return emission;
	}

	glm::vec3 diff_shade(const SurfaceInteraction & isect, const glm::vec3 & ob_pos);
	glm::vec3 spec_shade(const SurfaceInteraction & isect, const glm::vec3 & ob_pos, const glm::vec3 & view_dir);
	bool calc_shadow(glm::vec3 p, const Scene &sc);
	glm::vec3 phong_shade(const Scene & sc, const Ray & ray, const glm::vec3 & ob_pos, const SurfaceInteraction & isect);
};

struct PointLightShaped : public Light
{
	PointLightShaped(glm::vec3 p, glm::vec3 dir, glm::vec3 col) :
		Light(p, dir, col)
	{
	}

	// equal light emission in all directions
	glm::vec3 getEmission(glm::vec3 dir) const
	{
		return emission;
	}

	glm::vec3 diff_shade(const SurfaceInteraction & isect, const glm::vec3 & ob_pos);
	glm::vec3 spec_shade(const SurfaceInteraction & isect, const glm::vec3 & ob_pos, const glm::vec3 & view_dir);
	bool calc_shadow(glm::vec3 p, const Scene &sc);
	glm::vec3 phong_shade(const Scene & sc, const Ray & ray, const glm::vec3 & ob_pos, const SurfaceInteraction & isect);

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
	DistantLight(glm::vec3 dir, glm::vec3 col) :
		Light(glm::vec3(INFINITY), glm::normalize(dir), col)
	{
	}

	glm::vec3 getEmission(glm::vec3 dir) const
	{
		return emission;
	}
	glm::vec3 diff_shade(const SurfaceInteraction & isect, const glm::vec3 & ob_pos);
	glm::vec3 spec_shade(const SurfaceInteraction & isect, const glm::vec3 & ob_pos, const glm::vec3 & view_dir);
	bool calc_shadow(glm::vec3 p, const Scene &sc);
	glm::vec3 phong_shade(const Scene & sc, const Ray & ray, const glm::vec3 & ob_pos, const SurfaceInteraction & isect);
};
}