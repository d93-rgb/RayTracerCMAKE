#pragma once
#include "core/rt.h"
#include "texture/texture.h"

namespace rt
{
struct Material
{
	Material() :
		ambient(0),
		diffuse(0),
		specular(0),
		reflective(0),
		transparent(0),
		n(1.f),
		refr_indx(1.f),
		tex(nullptr)
	{
	}

	Material(glm::dvec3 amb, glm::dvec3 dif, glm::dvec3 spe, std::shared_ptr<Texture> tex = nullptr)
		:
		ambient(amb),
		diffuse(dif),
		specular(spe),
		reflective(0),
		transparent(0),
		n(1.f),
		refr_indx(1.f)
	{
		this->tex = tex;
	}

	glm::dvec3 getAmbient(glm::dvec3 pos);

	glm::dvec3 getDiffuse(glm::dvec3 pos);

	glm::dvec3 getSpecular()
	{
		return specular;
	}

	void setShininess(double exp)
	{
		n = exp;
	}

	double getShininess() const
	{
		return n;
	}

	void setReflective(glm::dvec3 r)
	{
		reflective = r;
	}

	glm::dvec3 getReflective()
	{
		return reflective ;
	}

	void setTransparent(glm::dvec3 t)
	{
		transparent = t;
	}

	glm::dvec3 getTransparent()
	{
		return transparent;
	}

	void setRefractiveIdx(double f)
	{
		this->refr_indx = f;
	}

	double getRefractiveIdx()
	{
		return refr_indx;
	}

	void setTexture(std::shared_ptr<Texture> tex)
	{
		this->tex = tex;
	}

protected:
	// specular exponent
	double n;
	glm::dvec3 ambient, diffuse, specular, reflective, transparent;
	double refr_indx;

	std::shared_ptr<Texture> tex;
};
}