#pragma once
#include "core/rt.h"

namespace rt
{
class TextureMapping
{
public:
	virtual glm::dvec2 getTextureCoordinates(const glm::dvec3 &pos) const = 0;

};

class SphericalMapping : public TextureMapping
{
	glm::dvec3 center;
public:
	SphericalMapping(glm::dvec3 center) : center(center)
	{
	}

	glm::dvec2 getTextureCoordinates(const glm::dvec3 &pos) const
	{
		glm::dvec3 pos_shift = glm::normalize(pos - center);
		//double radius = glm::length(pos_shift);
		double u = (1.0 + atan2(pos_shift.z, pos_shift.x) / M_PI) * 0.5;
		double v = acos(pos_shift.y) / M_PI;

		return glm::dvec2(u, v);
	}
};

class PlanarMapping : public TextureMapping
{
public:
	/*
		pos: plane position
		vs: spanning vector
		vt: (non parallel to vs) spanning vector
	*/
	PlanarMapping(glm::dvec3 pos, glm::dvec3 vs, glm::dvec3 vt) : 
		pos(pos), 
		vs(glm::normalize(vs)), 
		vt(glm::normalize(vt)),
		vsl(glm::length(vs)), 
		vtl(glm::length(vt))
	{}

	glm::dvec2 getTextureCoordinates(const glm::dvec3 &pos) const
	{
		double u = glm::dot(pos - this->pos, vs) / vsl;
		double v = glm::dot(pos - this->pos, vt) / vtl;

		return glm::dvec2(u, v);
	}

private:
	glm::dvec3 pos, vs, vt;
	double vsl, vtl;
};

} // namespace rt