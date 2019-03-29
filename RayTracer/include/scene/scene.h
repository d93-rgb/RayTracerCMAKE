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

	/*void emplace_back(std::unique_ptr<Shape> o)
	{
		sc.emplace_back(o);
	}
*/
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

class SingleCubeScene : public Scene
{
public:
	SingleCubeScene()
	{
		init();
	}

	void init();
};
}