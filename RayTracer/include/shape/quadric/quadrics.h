#pragma once

#include "core/rt.h"
#include "shape/shape.h"

namespace rt
{
class Quadric : public Shape
{

	static bool solveQuadraticEq();
};

struct Sphere : public Quadric
{
	float r;
	glm::vec3 origin;
	glm::vec3 color;

	Sphere(glm::vec3 origin, float radius, glm::vec3 color, std::shared_ptr<Material> m)
	{
		this->origin = origin;
		this->r = radius;
		this->color = color;
		this->mat = m;
	}

	glm::vec3 get_normal(glm::vec3 p) const
	{
		return glm::normalize(p - origin);
	}

	float intersect(const Ray &ray, SurfaceInteraction *isect);

};

struct Cylinder : public Quadric
{
	float height, radius;
	glm::vec3 pos, dir;
	glm::mat4 objToWorld;
	glm::mat4 worldToObj;
	glm::mat4 tr_worldToObj;

	Cylinder(glm::vec3 pos,
		glm::vec3 dir,
		float radius,
		float height,
		std::shared_ptr<Material> mat) :
		pos(pos), dir(glm::normalize(dir)), radius(radius), height(height)
	{
		this->mat = mat;

		glm::vec3 tangent_v = glm::normalize(Plane::getTangentVector(dir));

		//objToWorld = glm::lookAt(pos, pos + tangent_v, dir);
		// transform axis of the cylinder to the axis given by dir
		objToWorld[0] = glm::vec4(glm::cross(dir, tangent_v), 0.f);
		objToWorld[1] = glm::vec4(dir, 0.f);
		objToWorld[2] = glm::vec4(tangent_v, 0.f);
		objToWorld[3] = glm::vec4(pos, 1.f);

		worldToObj = glm::inverse(objToWorld);
		tr_worldToObj = glm::transpose(worldToObj);
	}

	float intersect(const Ray &ray, SurfaceInteraction *isect);

	glm::vec3 get_normal(glm::vec3 p, int hit_cnt) const
	{
		if (hit_cnt == 2)
			return glm::normalize(tr_worldToObj * glm::vec4(p.x, 0.f, p.z, 0.f));
		else
			return -glm::normalize((tr_worldToObj * glm::vec4(p.x, 0.f, p.z, 0.f)));
	}
};

// TODO: Implement the missing quadrics

struct Disk : public Quadric
{

};

struct Cone : public Quadric
{

};

struct Paraboloid : public Quadric
{

};

struct Hyperboloid : public Quadric
{

};


} // namespace rt