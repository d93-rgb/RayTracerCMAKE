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
	glm::vec3 p;
	glm::vec3 normal;
	glm::vec2 uv;
	std::shared_ptr<Material> mat;
	std::shared_ptr<Texture> texture;

	SurfaceInteraction() = default;
};
}