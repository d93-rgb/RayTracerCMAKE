#pragma once

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/perpendicular.hpp>

#include "core/rt.h"

namespace rt
{

class Scene
{
public:
	std::vector<std::unique_ptr<Shape>> sc;
	std::vector<std::unique_ptr<Light>> lights;

	std::unique_ptr<Camera> cam;

	Scene(size_t MAX_DEPTH) : MAX_DEPTH(MAX_DEPTH) {};

	double shoot_ray(const Ray& ray, SurfaceInteraction* isect);

	glm::dvec3 shoot_recursively(
		const Ray& ray,
		SurfaceInteraction* isect,
		int depth);

	const std::vector<std::unique_ptr<Shape>>& get_scene() const
	{
		return sc;
	}

	virtual void init() = 0;

protected:
	const size_t MAX_DEPTH;
};

class GatheringScene : public Scene
{
public:
	GatheringScene(size_t MAX_DEPTH = 4) :
		Scene(MAX_DEPTH)
	{
		init();
	}

	void init();
};

class MixedScene : public Scene
{
public:
	MixedScene(size_t MAX_DEPTH = 4) :
		Scene(MAX_DEPTH)
	{
		init();
	}

	void init();
};

class TeapotScene : public Scene
{
public:
	TeapotScene(size_t MAX_DEPTH = 4) :
		Scene(MAX_DEPTH)
	{
		init();
	}

	void init();
};

class SingleTriangleScene : public Scene
{
public:
	SingleTriangleScene(size_t MAX_DEPTH = 4) :
		Scene(MAX_DEPTH)
	{
		init();
	}

	void init();
};

class DragonScene : public Scene
{
public:
	DragonScene(size_t MAX_DEPTH = 4) :
		Scene(MAX_DEPTH)
	{
		init();
	}

	void init();
};

} // namespace rt