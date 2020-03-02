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
	const size_t MAX_DEPTH;

	std::vector<std::unique_ptr<Shape>> sc;
	std::vector<std::unique_ptr<Light>> lights;

	std::unique_ptr<Camera> cam;

	Scene(size_t MAX_DEPTH = 4);

	Scene(
		std::vector<std::unique_ptr<Shape>> sc,
		std::vector<std::unique_ptr<Light>> lights,
		size_t MAX_DEPTH = 4);

	double shoot_ray(const Ray& ray, SurfaceInteraction* isect) const;

	const std::vector<std::unique_ptr<Shape>>& get_scene() const
	{
		return sc;
	}

	virtual void init() = 0;

protected:
};

class GatheringScene : public Scene
{
public:
	GatheringScene(size_t MAX_DEPTH = 4);
	GatheringScene(
		std::vector<std::unique_ptr<Shape>> sc,
		std::vector<std::unique_ptr<Light>> lights, 
		size_t MAX_DEPTH = 4);

	void init();
};

class MixedScene : public Scene
{
public:
	MixedScene(size_t MAX_DEPTH = 4);
	MixedScene(
		std::vector<std::unique_ptr<Shape>> sc,
		std::vector<std::unique_ptr<Light>> lights,
		size_t MAX_DEPTH = 4);

	void init();
};

class TeapotScene : public Scene
{
public:
	TeapotScene(size_t MAX_DEPTH = 4);
	TeapotScene(
		std::vector<std::unique_ptr<Shape>> sc,
		std::vector<std::unique_ptr<Light>> lights,
		size_t MAX_DEPTH = 4);


	void init();
};

class SingleTriangleScene : public Scene
{
public:
	SingleTriangleScene(size_t MAX_DEPTH = 4);
	SingleTriangleScene(
		std::vector<std::unique_ptr<Shape>> sc,
		std::vector<std::unique_ptr<Light>> lights,
		size_t MAX_DEPTH = 4);

	void init();
};

class DragonScene : public Scene
{
public:
	DragonScene(size_t MAX_DEPTH = 4);
	DragonScene(
		std::vector<std::unique_ptr<Shape>> sc,
		std::vector<std::unique_ptr<Light>> lights,
		size_t MAX_DEPTH = 4);

	void init();
};

class TetrahedronScene : public Scene
{
public:
	double degree_step;

	TetrahedronScene(double degree_step, size_t MAX_DEPTH = 4);
	TetrahedronScene(
		std::vector<std::unique_ptr<Shape>> sc,
		std::vector<std::unique_ptr<Light>> lights,
		size_t MAX_DEPTH = 4);

	void init();
};

} // namespace rt