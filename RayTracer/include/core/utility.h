#pragma once
#include "core/rt.h"

inline double clamp(double f)
{
	return f < 0.f ? 0.f : (f > 1.f ? 1.f : f);
}

inline glm::dvec3 clamp(glm::dvec3 v)
{
	return glm::min(glm::dvec3(1.f), glm::max(glm::dvec3(0.f), v));
}

inline void crop(double min, double max, size_t x, size_t cropped[])
{
	cropped[0] = int(round(clamp(min) * x));
	cropped[1] = int(round(clamp(max) * x));
}

inline std::ostream& operator<<(std::ostream& os, glm::dvec3 v)
{
	os << "(" << v.x << ", " << v.y << ", " << v.z << ")" << std::endl;
	return os;
}