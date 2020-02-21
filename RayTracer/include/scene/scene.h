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

	Scene();

	const std::vector<std::unique_ptr<Shape>>& get_scene() const
	{
		return sc;
	}

	~Scene();

	virtual void init() = 0;

};

class GatheringScene : public Scene
{
public:
	GatheringScene()
	{
		init();
	}

	void init();
};

class MixedScene : public Scene
{
public:
	MixedScene()
	{
		init();
	}

	void init();
};

class TeapotScene : public Scene
{
public:
	TeapotScene()
	{
		init();
	}

	void init();
};

class SingleTriangleScene : public Scene
{
public:
	SingleTriangleScene()
	{
		init();
	}

	void init();
};

class DragonScene : public Scene
{
public:
	DragonScene()
	{
		init();
	}

	void init();
};

} // namespace rt