#pragma once
#include "core/rt.h"

inline float clamp(float f)
{
	return f < 0.f ? 0.f : (f > 1.f ? 1.f : f);
}

inline glm::vec3 clamp(glm::vec3 v)
{
	return glm::min(glm::vec3(1.f), glm::max(glm::vec3(0.f), v));
}

inline void crop(float min, float max, int x, int cropped[])
{
	cropped[0] = (int)round(clamp(min) * x);
	cropped[1] = (int)round(clamp(max) * x);
}