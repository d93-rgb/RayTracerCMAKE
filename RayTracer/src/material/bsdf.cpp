#pragma once
#include "material/bsdf.h"

namespace rt
{

RGB_Color BSDF::f(const glm::dvec3& wi, const glm::dvec3& wo) const
{
	return RGB_Color();
}

} // namespace rt