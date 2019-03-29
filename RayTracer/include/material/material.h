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

	Material(glm::vec3 amb, glm::vec3 dif, glm::vec3 spe, std::shared_ptr<Texture> tex = nullptr)
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

	glm::vec3 getAmbient(glm::vec3 pos);

	glm::vec3 getDiffuse(glm::vec3 pos);

	glm::vec3 getSpecular()
	{
		return specular;
	}

	void setShininess(float exp)
	{
		n = exp;
	}

	float getShininess() const
	{
		return n;
	}

	void setReflective(glm::vec3 r)
	{
		reflective = r;
	}

	glm::vec3 getReflective()
	{
		return reflective ;
	}

	void setTransparent(glm::vec3 t)
	{
		transparent = t;
	}

	glm::vec3 getTransparent()
	{
		return transparent;
	}

	void setRefractiveIdx(float f)
	{
		this->refr_indx = f;
	}

	float getRefractiveIdx()
	{
		return refr_indx;
	}

	void setTexture(std::shared_ptr<Texture> tex)
	{
		this->tex = tex;
	}

protected:
	// specular exponent
	float n;
	glm::vec3 ambient, diffuse, specular, reflective, transparent;
	float refr_indx;

	std::shared_ptr<Texture> tex;
};
}