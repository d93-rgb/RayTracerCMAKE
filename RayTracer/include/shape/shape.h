#pragma once
#include <glm/gtc/matrix_transform.hpp>

#include "core/rt.h"
#include "material/material.h"
#include "shape/ray.h"
#include "interaction/interaction.h"
#include "texture/texture.h"

//#define DEBUG

namespace rt
{

template <typename T> inline int sgn(T val)
{
	return (T(0) < val) - (val < T(0));
}

struct Shape
{
	glm::mat4 obj_to_world = glm::mat4(1.f);
	glm::mat4 world_to_obj = glm::mat4(1.f);
	//TODO: Decouple material interface from shape class for flexibility
	// => Take it over to the SurfaceInteraction class
	std::shared_ptr<Material> mat;

	virtual float intersect(const Ray &ray, SurfaceInteraction *isect) = 0;
};

class Bounds3
{
	glm::vec3 normal;
	glm::vec3 boundaries[2];

public:
	Bounds3(glm::vec3 min_bounds, glm::vec3 max_bounds)
	{
		for (int i = 0; i < 3; ++i)
		{
			assert(min_bounds[i] < max_bounds[i]);
		}

		boundaries[0] = (min_bounds);
		boundaries[1] = (max_bounds);
	}

	bool intersect(const Ray &ray)
	{
		assert(abs(length(ray.rd)) > 0);

		// no need for checking division by zero, floating point arithmetic is helping here
		glm::vec3 inv_rd = 1.f / ray.rd;
		glm::vec3 t[2] = { glm::vec3(INFINITY), glm::vec3(INFINITY) };
		// interval of intersection
		float t0 = 0.f, t1 = INFINITY;

		// the case where the ray is parallel to the plane is handled correctly by these two
		// calculations => if the ray is outside the slabs, the values will both be -/+ inf,
		// if it is inside, the values will be inf with different signs
		t[0] = boundaries[0] - ray.ro;
		t[1] = boundaries[1] - ray.ro;

		for (int i = 0; i < 3; ++i)
		{
			t[0][i] *= inv_rd[i];
			t[1][i] *= inv_rd[i];

			if (t[0][i] > t[1][i])
			{
				std::swap(t[0][i], t[1][i]);
			}

			// narrow interval and check if intersection possible
			// smaller interval bound may only get larger, the bigger interval bound may only 
			// become smaller
			t0 = t0 > t[0][i] ? t0 : t[0][i];
			t1 = t1 < t[1][i] ? t1 : t[1][i];

			if (t0 > t1)
			{
				return false;
			}
		}

		return true;
	}

	glm::vec3 get_normal(glm::vec3 p) const
	{
		return glm::vec3(0.f);
	}
};

struct Plane : public Shape
{
	glm::vec3 pos;
	glm::vec3 normal;
	glm::vec3 color;
	float k;

	Plane(glm::vec3 p, glm::vec3 n) :
		pos(p), normal(glm::normalize(n))
	{
		k = glm::dot(normal, pos);
	}

	Plane(glm::vec3 p, glm::vec3 n, glm::vec3 color, std::shared_ptr<Material> m) :
		pos(p), normal(glm::normalize(n)), color(color)
	{
		this->mat = m;
		k = glm::dot(normal, pos);
	}

	float intersect(const Ray &ray, SurfaceInteraction * isect);

	float intersect(const Ray &ray);

	/*
		Get the missing coordinate of the point P, so that it lies on the plane.

		coordinate = 0: get the missing x-coordinate => v = y and w = z
		coordinate = 1: get the missing y-coordinate => v = x and w = z
		coordinate = 2: get the missing z-coordinate => v = x and w = y
	*/
	glm::vec4 getPlanePos(float v, float w, int coordinate)
	{
		glm::vec4 c;
		float a;
		float tmp = glm::dot(normal, pos);

		switch (coordinate)
		{
		case 0:
			a = (tmp - (normal.y * v + normal.z * w)) / normal.x;
			c = glm::vec4(a, v, w, 1.f);
			break;
		case 1:
			a = (tmp - (normal.x * v + normal.z * w)) / normal.y;
			c = glm::vec4(v, a, w, 1.f);
			break;
		case 2:
			a = (tmp - (normal.x * v + normal.y * w)) / normal.z;
			c = glm::vec4(v, w, a, 1.f);
			break;
		default:
			return glm::vec4(INFINITY);
		}
		return c;
	}

	glm::vec3 get_normal(glm::vec3 p) const
	{
		return normal;
	}

	static glm::vec3 getTangentVector(glm::vec3 normal)
	{
		if (normal.x != 0.f)
		{
			return glm::vec3(-normal.y / normal.x, 1.f, 0.f);
		}
		else if (normal.y != 0.f)
		{
			return glm::vec3(1.f, -normal.x / normal.y, 0.f);
		}
		else if (normal.z != 0.f)
		{
			return glm::vec3(0.f, 1.f, -normal.y / normal.z);
		}

		return glm::vec3(0.f);
	}
};

struct Rectangle : public Shape
{
	glm::vec3 center;
	glm::vec3 v1;
	glm::vec3 v2;
	// normal of the plane the rectangle resides in
	glm::vec3 normal;

	float v1_dot;
	float v2_dot;

	Rectangle() {};

	Rectangle(glm::vec3 center, glm::vec3 u, glm::vec3 v, std::shared_ptr<Material> m) :
		center(center - 0.5f * (u + v))
	{
		this->mat = m;
		this->v1 = u;
		this->v2 = v;
		this->normal = glm::normalize(glm::cross(u, v));
		v1_dot = glm::dot(v1, v1);
		v2_dot = glm::dot(v2, v2);
	}

	/*
		Intersection routine for the transformed_ray-triangle intersection test.
		The nearest intersection parameter of Ray will not be updated
	*/
	float intersect(const Ray &ray, SurfaceInteraction * isect);

	/*
	Given two coordinates, this function calculates the missing third coordinate,
	so that the resulting point lies on the rectangle, if possible.
	Returns glm::vec3(INFINITY), if the point can not lie on the plane.
	*/
	glm::vec4 getRectPos(float v, float w, char coordinate)
	{
		glm::vec4 c;
		float a;
		float tmp = glm::dot(normal, center);

		switch (coordinate)
		{
		case 'x':
			//
			if (normal.x == 0.f)
			{
				if ((normal.y == 0.f || v == 0.f) && (normal.z == 0.f || w == 0.f))
				{
					return glm::vec4(INFINITY);
				}
				else
				{
					a = 0.f;
				}
			}
			else
			{
				a = (tmp - (normal.y * v + normal.z * w)) / normal.x;
			}
			c = glm::vec4(a, v, w, 1.f);
			break;
		case 'y':
			if (normal.y == 0.f)
			{
				if ((normal.x == 0.f || v == 0.f) && (normal.z == 0.f || w == 0.f))
				{
					return glm::vec4(INFINITY);
				}
				else
				{
					a = 0.f;
				}
			}
			else
			{
				a = (tmp - (normal.x * v + normal.z * w)) / normal.y;
			}
			c = glm::vec4(v, a, w, 1.f);
			break;
		case 'z':
			if (normal.z == 0.f)
			{
				if ((normal.x == 0.f || v == 0.f) && (normal.y == 0.f || w == 0.f))
				{
					return glm::vec4(INFINITY);
				}
				else
				{
					a = 0.f;
				}
			}
			else
			{
				a = (tmp - (normal.x * v + normal.y * w)) / normal.z;
			}
			c = glm::vec4(v, w, a, 1.f);
			break;
		default:
			return glm::vec4(INFINITY);
		}
		return c;
	}

	glm::vec3 get_normal(glm::vec3 p) const
	{
		return normal;
	}

	glm::vec3 get_normal() const
	{
		return normal;
	}
};

class UnitCube : public Shape
{
public:
	UnitCube(std::shared_ptr<Material> mat)
	{
		// cube must have thickness in all dimensios for now
		this->mat = mat;
	}

	float intersect(const Ray &ray, SurfaceInteraction *isect);

	/*
		Intersection test without updating the nearest intersection parameter for Ray.
		This routine will be used for the transformed_ray-bounding box intersection test.
	*/
	//float intersect(const Ray &ray);

	glm::vec3 get_normal(glm::vec3 p) const
	{
#ifdef DEBUG
		static int iter = 0;
#endif
#ifdef DEBUG
		if ((iter++) % 150 == 0) {
			bool stop = true;
		}
#endif
		p = world_to_obj * glm::vec4(p, 1.f);
		glm::vec3 a_p = glm::abs(p);
		glm::vec3 n = a_p.x > a_p.y ?
			(a_p.x > a_p.z ? glm::vec3(sgn(p.x), 0.f, 0.f) : glm::vec3(0.f, 0.f, sgn(p.z))) :
			(a_p.y > a_p.z ? glm::vec3(0.f, sgn(p.y), 0.f) : glm::vec3(0.f, 0.f, sgn(p.z)));
		return glm::normalize(glm::transpose(world_to_obj) * glm::vec4(n, 0.f));
	}

private:
	glm::vec3 boundaries{ 0.5f, 0.5f, 0.5f };

	friend class RGBCubeTexture;
};

class Cube : public Shape
{
public:
	Cube(glm::vec3 side_length, std::shared_ptr<Material> mat) :
		boundaries(side_length / 2.f)
	{
		// cube must have thickness in all dimensios for now
		assert(fmin(fmin(side_length.x, side_length.y), side_length.z) > 0);
		this->mat = mat;

		// sides
		v1[0] = glm::vec3(0.f, 0.f, side_length[2]);
		v2[0] = glm::vec3(0.f, side_length[1], 0.f);

		v1[1] = glm::vec3(side_length[0], 0.f, 0.f);
		v2[1] = glm::vec3(0.f, 0.f, side_length[2]);

		v1[2] = glm::vec3(side_length[0], 0.f, 0.f);
		v2[2] = glm::vec3(0.f, side_length[1], 0.f);

		for (int i = 0; i < 6; ++i)
		{
			// moved centers
			int i_m = i % 3;
			moved_centers[i] = side_length[i_m] * glm::vec3(1.f) *
				glm::vec3(i_m == 0, i_m == 1, i_m == 2) *
				(-1.f * (i >= 3 ? 1.f : -1.f)) - 0.5f * (v1[i_m] + v2[i_m]);
		}

		for (int i = 0; i < 3; ++i)
		{
			v1_dots[i] = glm::dot(v1[i], v1[i]);
			v2_dots[i] = glm::dot(v2[i], v2[i]);
		}
	}

	float intersect(const Ray &ray, SurfaceInteraction *isect);

	/*
		Intersection test without updating the nearest intersection parameter for Ray.
		This routine will be used for the transformed_ray-bounding box intersection test.
	*/
	float intersect(const Ray &ray);

	glm::vec3 get_normal(glm::vec3 p) const
	{
#ifdef DEBUG
		static int iter = 0;
#endif
#ifdef DEBUG
		if ((iter++) % 150 == 0) {
			bool stop = true;
		}
#endif
		p = world_to_obj * glm::vec4(p, 1.f);
		glm::vec3 a_p = glm::abs(p);
		glm::vec3 n = a_p.x > a_p.y ?
			(a_p.x > a_p.z ? glm::vec3(sgn(p.x), 0.f, 0.f) : glm::vec3(0.f, 0.f, sgn(p.z))) :
			(a_p.y > a_p.z ? glm::vec3(0.f, sgn(p.y), 0.f) : glm::vec3(0.f, 0.f, sgn(p.z)));
		return glm::normalize(glm::transpose(world_to_obj) * glm::vec4(n, 0.f));
	}

private:
	glm::vec3 normal;
	glm::vec3 boundaries;
	glm::vec3 moved_centers[6];
	glm::vec3 v1[3], v2[3];
	float v1_dots[3], v2_dots[3];

	friend class RGBCubeTexture;
};

class Triangle : public Shape
{
public:
	Triangle(glm::vec3 p1,
		glm::vec3 p2,
		glm::vec3 p3,
		glm::vec3 n,
		glm::mat4 objToWorld,
		std::shared_ptr<Material> mat) :
		objToWorld(objToWorld),
		worldToObj(glm::inverse(objToWorld))
	{
		this->mat = mat;

		this->p1 = objToWorld * glm::vec4(p1, 1.f);
		this->p2 = objToWorld * glm::vec4(p2, 1.f);
		this->p3 = objToWorld * glm::vec4(p3, 1.f);

		this->n = glm::transpose(glm::inverse(objToWorld)) * glm::vec4(n, 0.f);

		this->m_inv = glm::inverse(glm::mat3(this->p1, this->p2, this->p3));

	}

	float intersect(const Ray &ray, SurfaceInteraction *isect);

	glm::vec3 get_normal(glm::vec3 p) const
	{
		return n;
	}

	glm::vec3 get_normal() const
	{
		return n;
	}

private:
	// vertices
	glm::vec3 p1, p2, p3;
	// normal
	glm::vec3 n;
	glm::mat4 objToWorld;
	glm::mat4 worldToObj;
	glm::mat3 m_inv;

	friend class RGB_TextureTriangle;
};

class TriangleMesh : public Shape
{
public:
	std::unique_ptr<Bounds3> boundary;
	std::vector<std::unique_ptr<Triangle>> tr_mesh;

	TriangleMesh(std::unique_ptr<Bounds3> &bounds) : boundary(std::move(bounds))
	{
	}

	TriangleMesh() {}

	float intersect(const Ray &ray, SurfaceInteraction *isect)
	{
		float t_int = INFINITY;
		float tmp = INFINITY;
		bool hit = false;

		hit = boundary->intersect(ray);
		if (!hit) {
			return INFINITY;
		}

		// get nearest intersection point
		for (auto &objs : tr_mesh)
		{
			tmp = objs->intersect(ray, isect);

			if (tmp >= 0 && t_int > tmp)
			{
				t_int = tmp;
			}
		}
		return t_int;
	}

	glm::vec3 get_normal(glm::vec3 p) const
	{
		return glm::vec3(0.f);
	}

};

inline void create_cube(glm::vec3 center,
	glm::vec3 up,
	glm::vec3 front,
	float s_len,
	std::unique_ptr<Shape> sides[],
	std::shared_ptr<Material> mat)
{
	float tmp = s_len / 2;

	glm::vec3 n_up = glm::normalize(up);
	glm::vec3 n_front = glm::normalize(front);
	glm::vec3 n_u_cross_f = glm::normalize(glm::cross(n_up, n_front));

	glm::vec3 t_u = tmp * n_up;
	glm::vec3 t_f = tmp * n_front;
	glm::vec3 t_uf = tmp * n_u_cross_f;

	n_up = s_len * n_up;
	n_front = s_len * n_front;
	n_u_cross_f = s_len * n_u_cross_f;

	//top
	sides[0].reset(new Rectangle(center + t_u, -n_u_cross_f, n_front, mat));
	//bottom
	sides[1].reset(new Rectangle(center - t_u, n_front, -n_u_cross_f, mat));
	//front
	sides[2].reset(new Rectangle(center + t_f, n_up, -n_u_cross_f, mat));
	//back
	sides[3].reset(new Rectangle(center - t_f, -n_u_cross_f, n_up, mat));
	//left
	sides[4].reset(new Rectangle(center + t_uf, n_up, n_front, mat));
	//right
	sides[5].reset(new Rectangle(center - t_uf, n_front, n_up, mat));
}

}