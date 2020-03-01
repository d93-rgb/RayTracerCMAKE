#pragma once
#include "core/rt.h"

namespace rt
{

class BSDF
{
public:
	RGB_Color f(const glm::dvec3& wi, const glm::dvec3& wo) const;
};

} // namespace rt