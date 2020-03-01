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

	virtual RGB_Color sample_light(const glm::dvec3 isect_p, glm::dvec3& dir_to_light, double& pdf) = 0;

	virtual bool visible(const glm::dvec3& p, const Scene &sc) const = 0 ;
};

struct PointLight : public Light
{
	PointLight(glm::dvec3 p, glm::dvec3 dir, glm::dvec3 col) :
		Light(p, dir, col)
	{
		intensity = col;
	}

	// equal light emission in all directions
	glm::dvec3 getEmission(glm::dvec3 dir) const
	{
		return emission;
	}

	RGB_Color sample_light(const glm::dvec3 isect_p, glm::dvec3& dir_to_light, double& pdf);

	bool visible(const glm::dvec3& p, const Scene &sc) const;

private:
	RGB_Color intensity; // dimension: [W/m^2]
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

	bool visible(const glm::dvec3& p, const Scene &sc) const;
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

	bool visible(const glm::dvec3& p, const Scene &sc) const;
};
}