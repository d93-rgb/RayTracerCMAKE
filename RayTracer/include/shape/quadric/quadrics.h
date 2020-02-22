#pragma once

#include "core/rt.h"
#include "shape/shape.h"

namespace rt
{
class Quadric : public Shape
{
public:
	static bool solveQuadraticEq(double* t, double a, double b, double c);
};

struct Sphere : public Quadric
{
	double r;
	glm::dvec3 origin;
	glm::dvec3 color;

	Sphere(glm::dvec3 origin, double radius, glm::dvec3 color, std::shared_ptr<Material> m)
	{
		this->origin = origin;
		this->r = radius;
		this->color = color;
		this->mat = m;
	}

	glm::dvec3 get_normal(glm::dvec3 p) const
	{
		return glm::normalize(p - origin);
	}

	double intersect(const Ray &ray, SurfaceInteraction *isect);

};

struct Cylinder : public Quadric
{
	double height, radius;
	glm::dvec3 pos, dir;
	glm::dmat4 objToWorld;
	glm::dmat4 worldToObj;
	glm::dmat4 tr_worldToObj;

	Cylinder(glm::dvec3 pos,
		glm::dvec3 dir,
		double radius,
		double height,
		std::shared_ptr<Material> mat) :
		pos(pos), dir(glm::normalize(dir)), radius(radius), height(height)
	{
		this->mat = mat;

		glm::dvec3 tangent_v = glm::normalize(Plane::getTangentVector(dir));

		//objToWorld = glm::lookAt(pos, pos + tangent_v, dir);
		// transform axis of the cylinder to the axis given by dir
		objToWorld[0] = glm::dvec4(glm::cross(dir, tangent_v), 0.f);
		objToWorld[1] = glm::dvec4(dir, 0.f);
		objToWorld[2] = glm::dvec4(tangent_v, 0.f);
		objToWorld[3] = glm::dvec4(pos, 1.f);

		worldToObj = glm::inverse(objToWorld);
		tr_worldToObj = glm::transpose(worldToObj);
	}

	double intersect(const Ray &ray, SurfaceInteraction *isect);

	glm::dvec3 get_normal(glm::dvec3 p, int hit_cnt) const
	{
		if (hit_cnt == 2)
			return glm::normalize(tr_worldToObj * glm::dvec4(p.x, 0.f, p.z, 0.f));
		else
			return -glm::normalize((tr_worldToObj * glm::dvec4(p.x, 0.f, p.z, 0.f)));
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