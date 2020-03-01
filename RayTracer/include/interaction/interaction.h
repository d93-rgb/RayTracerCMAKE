#pragma once
#include "core/rt.h"
#include "material/material.h"

namespace rt
{
class Interaction
{

};

class SurfaceInteraction : public Interaction
{
public:
	glm::dvec3 p;
	glm::dvec3 normal;
	glm::dvec2 uv;
	std::shared_ptr<Material> mat;
	std::shared_ptr<Texture> texture;
	std::shared_ptr<BSDF> bsdf;

	SurfaceInteraction() :
		p(0), normal(0), uv(0){}
};
}