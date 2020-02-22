#pragma once
#include "core/rt.h"

namespace rt
{
struct Ray
{
	glm::dvec3 ro;
	glm::dvec3 rd;
	mutable double tNearest;

	Ray() : ro(0), rd(0), tNearest(INFINITY) {}

	Ray(glm::dvec3 ro, glm::dvec3 rd) :
		ro(ro), rd(rd), tNearest(INFINITY)
	{
	}
	Ray(glm::dvec3 ro, glm::dvec3 rd, double tNearest) :
		ro(ro), rd(rd), tNearest(tNearest)
	{
	}
};
}