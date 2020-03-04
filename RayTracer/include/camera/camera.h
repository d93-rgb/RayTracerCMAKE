#pragma once
#include "core/rt.h"
#include "shape/ray.h"

namespace rt
{
class Camera
{
public:
	Camera() : origin(glm::dvec4(0.0, 0.0, 0.0, 1.0)),
		right(glm::dvec4(1.0, 0.0, 0.0, 0.0)),
		up(glm::dvec4(0.0, 1.0, 0.0, 0.0)),
		front(glm::dvec4(0.0, 0.0, 1.0, 0.0)),
		camToWorld(1.0)
	{
	}

	Camera(glm::dvec4 o, glm::dvec4 up, glm::dvec4 right, glm::dvec4 front) :
		origin(o),
		up(glm::normalize(up)),
		right(glm::normalize(right)),
		front(glm::normalize(front)),
		camToWorld(1.0)
	{
	}

	~Camera();
	
	void setCamToWorld(glm::dvec3 eyePosition, glm::dvec3 gazePoint, glm::dvec3 upVector);

	virtual Ray getPrimaryRay(double u, double v, double d)
	{
		auto dir = glm::normalize((u * right + v * up - d * front));
		return Ray(origin, dir);
	}

	glm::dvec4 getOrigin()
	{
		return origin;
	}

	glm::dvec4 getUpVec()	
	{
		return up;
	}

	void update();

protected:
	glm::dvec4 origin;
	glm::dvec4 up, right, front;
	glm::dmat4 camToWorld;
};

// orthographic projection camera
class OrthographicCamera : public Camera
{
public:
	OrthographicCamera() : origin(glm::dvec4(0.0, 0.0, 0.0, 1.0)),
		right(glm::dvec4(1.0, 0.0, 0.0, 0.0)),
		up(glm::dvec4(0.0, 1.0, 0.0, 0.0)),
		front(glm::dvec4(0.0, 0.0, 1.0, 0.0)),
		camToWorld(1.0)
	{
	};

	Ray getPrimaryRay(double u, double v, double d) override
	{
		return Ray(15.0*u * right + 15.0*v * up, -front);
	}

	glm::dvec4 getOrigin()
	{
		return origin;
	}

	glm::dvec4 getUpVec()
	{
		return up;
	}

	void update();

protected:
	glm::dvec4 origin;
	glm::dvec4 up, right, front;
	glm::dmat4 camToWorld;
};
}