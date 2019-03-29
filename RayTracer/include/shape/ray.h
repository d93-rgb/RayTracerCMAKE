#pragma once
#include "core/rt.h"

namespace rt
{
struct Ray
{
	glm::vec3 ro;
	glm::vec3 rd;
	mutable float tNearest;

	Ray() : ro(0), rd(0) {}

	Ray(glm::vec3 ro, glm::vec3 rd) :
		ro(ro), rd(rd), tNearest(INFINITY)
	{
	}
	Ray(glm::vec3 ro, glm::vec3 rd, float tNearest) :
		ro(ro), rd(rd), tNearest(tNearest)
	{
	}
};
}