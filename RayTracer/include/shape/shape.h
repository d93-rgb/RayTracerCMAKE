#pragma once
#include <glm/gtc/matrix_transform.hpp>

#include "core/rt.h"
#include "material/material.h"
#include "shape/ray.h"
#include "interaction/interaction.h"
#include "texture/texture.h"
#include "shape/bvh.h"

//#define DEBUG

namespace rt
{

template <typename T> inline int sgn(T val)
{
	return (T(0) < val) - (val < T(0));
}

struct Shape
{
	glm::dmat4 obj_to_world = glm::dmat4(1.f);
	glm::dmat4 world_to_obj = glm::dmat4(1.f);
	//TODO: Decouple material interface from shape class for flexibility
	// => Take it over to the SurfaceInteraction class
	std::shared_ptr<Material> mat;

	virtual double intersect(const Ray &ray, SurfaceInteraction *isect) = 0;

	std::unique_ptr<Bounds3> bounding_box;
};

class Bounds3
{
	glm::dvec3 normal;

public:
	glm::dvec3 boundaries[2];
	glm::dvec3 centroid;

	Bounds3() = default;

	Bounds3(glm::dvec3 min_bounds, glm::dvec3 max_bounds) :
		normal(glm::dvec3(0.0)), centroid(0.5 * (min_bounds + max_bounds))
	{
		for (int i = 0; i < 3; ++i)
		{
			assert(min_bounds[i] <= max_bounds[i]);
		}

		boundaries[0] = (min_bounds);
		boundaries[1] = (max_bounds);
	}

	double intersect(const Ray &ray)
	{
		assert(abs(length(ray.rd)) > 0);

		// no need for checking division by zero, doubleing point arithmetic is helping here
		glm::dvec3 inv_rd = 1.0 / ray.rd;
		glm::dvec3 t[2] = { glm::dvec3(INFINITY), glm::dvec3(INFINITY) };
		// interval of intersection
		double t0 = 0.0, t1 = INFINITY;

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
				return INFINITY;
			}
		}

		return t0;
	}

	glm::dvec3 get_normal(glm::dvec3 p) const
	{
		return glm::dvec3(0.f);
	}
};

struct Plane : public Shape
{
	glm::dvec3 pos;
	glm::dvec3 normal;
	glm::dvec3 color;
	double k;

	Plane(glm::dvec3 p, glm::dvec3 n) :
		pos(p), normal(glm::normalize(n)), color(glm::dvec3(0))
	{
		k = glm::dot(normal, pos);
	}

	Plane(glm::dvec3 p, glm::dvec3 n, glm::dvec3 color, std::shared_ptr<Material> m) :
		pos(p), normal(glm::normalize(n)), color(color)
	{
		this->mat = m;
		k = glm::dot(normal, pos);
	}

	double intersect(const Ray &ray, SurfaceInteraction * isect);

	double intersect(const Ray &ray);

	/*
		Get the missing coordinate of the point P, so that it lies on the plane.

		coordinate = 0: get the missing x-coordinate => v = y and w = z
		coordinate = 1: get the missing y-coordinate => v = x and w = z
		coordinate = 2: get the missing z-coordinate => v = x and w = y
	*/
	glm::dvec4 getPlanePos(double v, double w, int coordinate)
	{
		glm::dvec4 c;
		double a;
		double tmp = glm::dot(normal, pos);

		switch (coordinate)
		{
		case 0:
			a = (tmp - (normal.y * v + normal.z * w)) / normal.x;
			c = glm::dvec4(a, v, w, 1.f);
			break;
		case 1:
			a = (tmp - (normal.x * v + normal.z * w)) / normal.y;
			c = glm::dvec4(v, a, w, 1.f);
			break;
		case 2:
			a = (tmp - (normal.x * v + normal.y * w)) / normal.z;
			c = glm::dvec4(v, w, a, 1.f);
			break;
		default:
			return glm::dvec4(INFINITY);
		}
		return c;
	}

	glm::dvec3 get_normal(glm::dvec3 p) const
	{
		return normal;
	}

	static glm::dvec3 getTangentVector(glm::dvec3 normal)
	{
		if (normal.x != 0.f)
		{
			return glm::dvec3(-normal.y / normal.x, 1.f, 0.f);
		}
		else if (normal.y != 0.f)
		{
			return glm::dvec3(1.f, -normal.x / normal.y, 0.f);
		}
		else if (normal.z != 0.f)
		{
			return glm::dvec3(0.f, 1.f, -normal.y / normal.z);
		}

		return glm::dvec3(0.f);
	}
};

struct Rectangle : public Shape
{
	glm::dvec3 center;
	glm::dvec3 v1;
	glm::dvec3 v2;
	// normal of the plane the rectangle resides in
	glm::dvec3 normal;

	double v1_dot = 0;
	double v2_dot = 0;

	Rectangle() = default;

	Rectangle(glm::dvec3 center, glm::dvec3 u, glm::dvec3 v, std::shared_ptr<Material> m) :
		center(center - 0.5 * (u + v))
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
	double intersect(const Ray &ray, SurfaceInteraction * isect);

	/*
	Given two coordinates, this function calculates the missing third coordinate,
	so that the resulting point lies on the rectangle, if possible.
	Returns glm::dvec3(INFINITY), if the point can not lie on the plane.
	*/
	glm::dvec4 getRectPos(double v, double w, char coordinate)
	{
		glm::dvec4 c;
		double a;
		double tmp = glm::dot(normal, center);

		switch (coordinate)
		{
		case 'x':
			//
			if (normal.x == 0.f)
			{
				if ((normal.y == 0.f || v == 0.f) && (normal.z == 0.f || w == 0.f))
				{
					return glm::dvec4(INFINITY);
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
			c = glm::dvec4(a, v, w, 1.f);
			break;
		case 'y':
			if (normal.y == 0.f)
			{
				if ((normal.x == 0.f || v == 0.f) && (normal.z == 0.f || w == 0.f))
				{
					return glm::dvec4(INFINITY);
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
			c = glm::dvec4(v, a, w, 1.f);
			break;
		case 'z':
			if (normal.z == 0.f)
			{
				if ((normal.x == 0.f || v == 0.f) && (normal.y == 0.f || w == 0.f))
				{
					return glm::dvec4(INFINITY);
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
			c = glm::dvec4(v, w, a, 1.f);
			break;
		default:
			return glm::dvec4(INFINITY);
		}
		return c;
	}

	glm::dvec3 get_normal(glm::dvec3 p) const
	{
		return normal;
	}

	glm::dvec3 get_normal() const
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

	double intersect(const Ray &ray, SurfaceInteraction *isect);

	/*
		Intersection test without updating the nearest intersection parameter for Ray.
		This routine will be used for the transformed_ray-bounding box intersection test.
	*/
	//double intersect(const Ray &ray);

	glm::dvec3 get_normal(glm::dvec3 p) const
	{
#ifdef DEBUG
		static int iter = 0;
#endif
#ifdef DEBUG
		if ((iter++) % 150 == 0) {
			bool stop = true;
		}
#endif
		p = world_to_obj * glm::dvec4(p, 1.f);
		glm::dvec3 a_p = glm::abs(p);
		glm::dvec3 n = a_p.x > a_p.y ?
			(a_p.x > a_p.z ? glm::dvec3(sgn(p.x), 0.f, 0.f) : glm::dvec3(0.f, 0.f, sgn(p.z))) :
			(a_p.y > a_p.z ? glm::dvec3(0.f, sgn(p.y), 0.f) : glm::dvec3(0.f, 0.f, sgn(p.z)));
		return glm::normalize(glm::transpose(world_to_obj) * glm::dvec4(n, 0.f));
	}

private:
	glm::dvec3 boundaries{ 0.5f, 0.5f, 0.5f };

	friend class RGBCubeTexture;
};

class Cube : public Shape
{
public:
	Cube(glm::dvec3 side_length, std::shared_ptr<Material> mat) :
		boundaries(side_length / 2.0)
	{
		// cube must have thickness in all dimensios for now
		assert(fmin(fmin(side_length.x, side_length.y), side_length.z) > 0);
		this->mat = mat;

		// sides
		v1[0] = glm::dvec3(0.f, 0.f, side_length[2]);
		v2[0] = glm::dvec3(0.f, side_length[1], 0.f);

		v1[1] = glm::dvec3(side_length[0], 0.f, 0.f);
		v2[1] = glm::dvec3(0.f, 0.f, side_length[2]);

		v1[2] = glm::dvec3(side_length[0], 0.f, 0.f);
		v2[2] = glm::dvec3(0.f, side_length[1], 0.f);

		for (int i = 0; i < 6; ++i)
		{
			// moved centers
			int i_m = i % 3;
			moved_centers[i] = side_length[i_m] * glm::dvec3(1.0) *
				glm::dvec3(i_m == 0, i_m == 1, i_m == 2) *
				(-1.0 * (i >= 3 ? 1.0 : -1.0)) - 0.5 * (v1[i_m] + v2[i_m]);
		}

		for (int i = 0; i < 3; ++i)
		{
			v1_dots[i] = glm::dot(v1[i], v1[i]);
			v2_dots[i] = glm::dot(v2[i], v2[i]);
		}
	}

	double intersect(const Ray &ray, SurfaceInteraction *isect);

	/*
		Intersection test without updating the nearest intersection parameter for Ray.
		This routine will be used for the transformed_ray-bounding box intersection test.
	*/
	double intersect(const Ray &ray);

	glm::dvec3 get_normal(glm::dvec3 p) const
	{
#ifdef DEBUG
		static int iter = 0;
#endif
#ifdef DEBUG
		if ((iter++) % 150 == 0) {
			bool stop = true;
		}
#endif
		p = world_to_obj * glm::dvec4(p, 1.f);
		glm::dvec3 a_p = glm::abs(p);
		glm::dvec3 n = a_p.x > a_p.y ?
			(a_p.x > a_p.z ? glm::dvec3(sgn(p.x), 0.f, 0.f) : glm::dvec3(0.f, 0.f, sgn(p.z))) :
			(a_p.y > a_p.z ? glm::dvec3(0.f, sgn(p.y), 0.f) : glm::dvec3(0.f, 0.f, sgn(p.z)));
		return glm::normalize(glm::transpose(world_to_obj) * glm::dvec4(n, 0.f));
	}

private:
	glm::dvec3 normal;
	glm::dvec3 boundaries;
	glm::dvec3 moved_centers[6];
	glm::dvec3 v1[3], v2[3];
	double v1_dots[3], v2_dots[3];

	friend class RGBCubeTexture;
};

class Triangle : public Shape
{
public:

	Triangle(glm::dvec3 p1,
		glm::dvec3 p2,
		glm::dvec3 p3,
		glm::dvec3 n1,
		glm::dvec3 n2,
		glm::dvec3 n3,
		glm::dvec3 n,
		glm::dmat4 objToWorld,
		std::shared_ptr<Material> mat) :
		objToWorld(objToWorld),
		worldToObj(glm::inverse(objToWorld))
	{
		this->mat = mat;

		this->p1 = objToWorld * glm::dvec4(p1, 1.f);
		this->p2 = objToWorld * glm::dvec4(p2, 1.f);
		this->p3 = objToWorld * glm::dvec4(p3, 1.f);

		//construct surrounding aabb
		this->bounding_box = std::make_unique<Bounds3>(glm::dvec3(glm::min(glm::min(this->p1, this->p2), this->p3)),
			glm::dvec3(glm::max(glm::max(this->p1, this->p2), this->p3)));

		this->n1 = glm::transpose(worldToObj) * glm::dvec4(n1, 0.f);
		this->n2 = glm::transpose(worldToObj) * glm::dvec4(n2, 0.f);
		this->n3 = glm::transpose(worldToObj) * glm::dvec4(n3, 0.f);

		this->plane_normal = glm::normalize(glm::transpose(worldToObj) * glm::dvec4(n, 0.f));

		// base transformation to barycentric coordinates
		// see: https://de.wikipedia.org/wiki/Basiswechsel_(Vektorraum)
		this->m_inv = glm::inverse(glm::dmat3(this->p1, this->p2, this->p3));
	}

	double intersect(const Ray &ray, SurfaceInteraction *isect);

	glm::dvec3 get_normal(glm::dvec3 p) const
	{
		// flat shading
		//return plane_normal;
		glm::dvec3 barycentric_coord = m_inv * p;
		assert(glm::all(glm::lessThanEqual(barycentric_coord, glm::dvec3(1))));

		return glm::normalize(barycentric_coord.x * n1 + barycentric_coord.y * n2 + 
			barycentric_coord.z * n3);
	}

	void set_objToWorld(const glm::dmat4& objToWorld)
	{
		this->objToWorld = objToWorld;
		this->worldToObj = glm::inverse(objToWorld);

		p1 = objToWorld * glm::dvec4(p1, 1.f);
		p2 = objToWorld * glm::dvec4(p2, 1.f);
		p3 = objToWorld * glm::dvec4(p3, 1.f);

		//construct surrounding aabb
		bounding_box = std::make_unique<Bounds3>(glm::dvec3(glm::min(glm::min(p1, p2), p3)),
			glm::dvec3(glm::max(glm::max(p1, p2), p3)));

		n1 = glm::transpose(worldToObj) * glm::dvec4(n1, 0.f);
		n2 = glm::transpose(worldToObj) * glm::dvec4(n2, 0.f);
		n3 = glm::transpose(worldToObj) * glm::dvec4(n3, 0.f);

		plane_normal = glm::normalize(glm::transpose(worldToObj) * glm::dvec4(plane_normal, 0.f));

		// base transformation to barycentric coordinates
		// see: https://de.wikipedia.org/wiki/Basiswechsel_(Vektorraum)
		this->m_inv = glm::inverse(glm::dmat3(p1, p2, p3));
	}

	void set_material(std::shared_ptr<Material> mat)
	{
		this->mat = std::move(mat);
	}

	//deprecated, replace with get_normal(glm::dvec3)
	/*glm::dvec3 get_normal() const
	{
		return n1;
	}*/

	Bounds3* get_bounding_box()
	{
		return bounding_box.get();
	}

private:
	// vertices
	glm::dvec3 p1, p2, p3;
	// normal
	glm::dvec3 n1, n2, n3;
	glm::dvec3 plane_normal;
	glm::dmat4 objToWorld;
	glm::dmat4 worldToObj;
	glm::dmat3 m_inv;

	friend class RGB_TextureTriangle;
};

class TriangleMesh : public Shape
{
public:
	std::unique_ptr<Bounds3> boundary;
	std::vector<std::shared_ptr<Shape>> tr_mesh;

	TriangleMesh(std::vector<std::shared_ptr<Shape>> tr_mesh) :
		tr_mesh(tr_mesh)
	{
		bvh = std::make_unique<BVH>(tr_mesh);
	}

	double intersect(const Ray& ray, SurfaceInteraction* isect);

	glm::dvec3 get_normal(glm::dvec3 p) const
	{
		return glm::dvec3(0.f);
	}
private:
	std::unique_ptr<BVH> bvh;
};

inline void create_cube(glm::dvec3 center,
	glm::dvec3 up,
	glm::dvec3 front,
	double s_len,
	std::unique_ptr<Shape> sides[],
	std::shared_ptr<Material> mat)
{
	double tmp = s_len / 2;

	glm::dvec3 n_up = glm::normalize(up);
	glm::dvec3 n_front = glm::normalize(front);
	glm::dvec3 n_u_cross_f = glm::normalize(glm::cross(n_up, n_front));

	glm::dvec3 t_u = tmp * n_up;
	glm::dvec3 t_f = tmp * n_front;
	glm::dvec3 t_uf = tmp * n_u_cross_f;

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