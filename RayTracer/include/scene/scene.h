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

	Scene(
		std::vector<std::unique_ptr<Shape>> sc,
		std::vector<std::unique_ptr<Light>> lights,
		size_t MAX_DEPTH = 4);

	double shoot_ray(const Ray& ray, SurfaceInteraction* isect);

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
	GatheringScene(
		std::vector<std::unique_ptr<Shape>> sc,
		std::vector<std::unique_ptr<Light>> lights, 
		size_t MAX_DEPTH = 4);

	void init();
};

class MixedScene : public Scene
{
public:
	MixedScene(
		std::vector<std::unique_ptr<Shape>> sc,
		std::vector<std::unique_ptr<Light>> lights,
		size_t MAX_DEPTH = 4);

	void init();
};

class TeapotScene : public Scene
{
public:
	TeapotScene(
		std::vector<std::unique_ptr<Shape>> sc,
		std::vector<std::unique_ptr<Light>> lights,
		size_t MAX_DEPTH = 4);


	void init();
};

class SingleTriangleScene : public Scene
{
public:
	SingleTriangleScene(
		std::vector<std::unique_ptr<Shape>> sc,
		std::vector<std::unique_ptr<Light>> lights,
		size_t MAX_DEPTH = 4);

	void init();
};

class DragonScene : public Scene
{
public:
	DragonScene(
		std::vector<std::unique_ptr<Shape>> sc,
		std::vector<std::unique_ptr<Light>> lights,
		size_t MAX_DEPTH = 4);

	void init();
};

} // namespace rt