#pragma once
#include "core/rt.h"
#include "interaction/interaction.h"

extern int MAX_DEPTH;

namespace rt
{
float shoot_ray(const Scene &s, const Ray &ray, SurfaceInteraction *isect);

glm::vec3 shoot_recursively(const Scene &s,
	const Ray &ray,
	SurfaceInteraction *isect,
	int depth);

glm::vec3 handle_transmission(const Scene &s,
	const Ray &ray,
	const glm::vec3 &isect_p,
	SurfaceInteraction *isect,
	int depth);

glm::vec3 handle_reflection(const Scene &s,
	const Ray &ray,
	const glm::vec3 &isect_p,
	SurfaceInteraction *isect,
	int depth);
}