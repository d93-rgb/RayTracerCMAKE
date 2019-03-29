#include "shape/shape.h"

namespace rt
{
float Plane::intersect(const Ray &ray, SurfaceInteraction * isect)
{
	float denom = glm::dot(normal, ray.rd);

	if (abs(denom) < 1e-6) return INFINITY;

	float num = k - glm::dot(normal, ray.ro);

	float t = num / denom;

	//if(t >= 0) std::cout << t << std::endl;
	t = t >= 0 ? t : INFINITY;

	if (t >= 0 && t < INFINITY)
	{
		if (t < ray.tNearest)
		{
			ray.tNearest = t;
			isect->p = ray.ro + ray.rd * t;
			isect->normal = get_normal(isect->p);
			isect->mat = mat;
		}
	}

	return t;
}

float Plane::intersect(const Ray &ray)
{
	float denom = glm::dot(normal, ray.rd);

	if (abs(denom) < 1e-6) return INFINITY;

	float num = k - glm::dot(normal, ray.ro);

	float t = num / denom;

	return t >= 0 ? t : INFINITY;
}

float Rectangle::intersect(const Ray &ray, SurfaceInteraction * isect)
{
	float denom = glm::dot(ray.rd, normal);

	if (abs(denom) < 1e-6) return INFINITY;

	float num = glm::dot(normal, center - ray.ro);

	float t = num / denom;

	if (t < 0) return INFINITY;

	glm::vec3 isec_p = ray.ro + t * ray.rd;

	float inside_1 = glm::dot(isec_p - center, v1) / v1_dot;
	float inside_2 = glm::dot(isec_p - center, v2) / v2_dot;

	bool test = (0 <= inside_1) && (inside_1 <= 1) &&
		(0 <= inside_2) && (inside_2 <= 1);

	if (test)
	{
		if (t < ray.tNearest)
		{
			ray.tNearest = t;
			isect->p = isec_p;
			isect->normal = get_normal(isect->p);
			isect->mat = mat;
		}
	}
	return test ? t : INFINITY;;

}

float Cube::intersect(const Ray &ray, SurfaceInteraction *isect)
{
	assert(abs(length(ray.rd)) > 0);

	Ray transformed_ray{ world_to_obj * glm::vec4(ray.ro, 1.f),
		world_to_obj * glm::vec4(ray.rd, 0.f) };

	int nearest = 0;
	int tmp = 0;
	float isec_t = INFINITY;
	float inside_1;
	float inside_2;
	bool tests[6] = { false };
	glm::vec3 t[2] = { glm::vec3(INFINITY), glm::vec3(INFINITY) };
	glm::vec3 isec_p;


	// calculate the 6 transformed_ray-plane intersections
	t[0] = (boundaries - transformed_ray.ro);
	t[1] = -(boundaries + transformed_ray.ro);

	// not regarding IEEE floating point arithmetics here, see Bounds3 for a better variant
	if (transformed_ray.rd.x != 0)
	{
		t[0].x /= transformed_ray.rd.x;
		t[1].x /= transformed_ray.rd.x;
	}
	else
	{
		t[0].x = (t[0].x == 0 ? 0.f : INFINITY);
		t[1].x = (t[1].x == 0 ? 0.f : INFINITY);
	}

	if (transformed_ray.rd.y != 0)
	{
		t[0].y /= transformed_ray.rd.y;
		t[1].y /= transformed_ray.rd.y;
	}
	else
	{
		t[0].y = (t[0].y == 0 ? 0.f : INFINITY);
		t[1].y = (t[1].y == 0 ? 0.f : INFINITY);
	}

	if (transformed_ray.rd.z != 0)
	{
		t[0].z /= transformed_ray.rd.z;
		t[1].z /= transformed_ray.rd.z;
	}
	else
	{
		t[0].z = (t[0].z == 0 ? 0.f : INFINITY);
		t[1].z = (t[1].z == 0 ? 0.f : INFINITY);
	}

	// check if inside boundaries
	for (int i = 0; i < 6; ++i)
	{
		tmp = i % 3;
		// filter out negative parameters
		if (t[i >= 3][tmp] >= 0.f)
		{
			isec_t = t[i >= 3][tmp];
			isec_p = transformed_ray.ro + isec_t * transformed_ray.rd;

			// project onto spanning vectors of the plane
			inside_1 = glm::dot(isec_p - moved_centers[i], v1[tmp]) /
				v1_dots[tmp];
			inside_2 = glm::dot(isec_p - moved_centers[i], v2[tmp]) /
				v2_dots[tmp];

			tests[i] = (0 <= inside_1) && (inside_1 <= 1)
				&& (0 <= inside_2) && (inside_2 <= 1);
		}
	}

	// get smallest positive parameter
	isec_t = INFINITY;
	for (int i = 0; i < 6; ++i)
	{
		tmp = i % 3;
		if (tests[i])
		{
			if (isec_t > t[i >= 3][tmp])
			{
				isec_t = t[i >= 3][tmp];
			}
		}
	}

	if (isec_t >= 0 && isec_t < INFINITY)
	{
		if (isec_t < ray.tNearest)
		{
			// update maximum intersection parameter
			ray.tNearest = isec_t;
			// update intersection properties
			isect->p = ray.ro + ray.rd * isec_t;
			isect->normal = get_normal(isect->p);
			isect->mat = mat;
		}
	}

	return isec_t;
}

float Cube::intersect(const Ray &ray)
{
	assert(abs(length(ray.rd)) > 0);

	Ray transformed_ray{ world_to_obj * glm::vec4(ray.ro, 1.f),
		world_to_obj * glm::vec4(ray.rd, 0.f) };

	int nearest = 0;
	int tmp = 0;
	float isec_t = INFINITY;
	float inside_1;
	float inside_2;
	bool tests[6] = { false };
	glm::vec3 t[2] = { glm::vec3(INFINITY), glm::vec3(INFINITY) };
	glm::vec3 isec_p;


	// calculate the 6 transformed_ray-plane intersections
	t[0] = (boundaries - transformed_ray.ro);
	t[1] = -(boundaries + transformed_ray.ro);


	if (transformed_ray.rd.x != 0)
	{
		t[0].x /= transformed_ray.rd.x;
		t[1].x /= transformed_ray.rd.x;
	}
	else
	{
		t[0].x = (t[0].x == 0 ? 0.f : INFINITY);
		t[1].x = (t[1].x == 0 ? 0.f : INFINITY);
	}

	if (transformed_ray.rd.y != 0)
	{
		t[0].y /= transformed_ray.rd.y;
		t[1].y /= transformed_ray.rd.y;
	}
	else
	{
		t[0].y = (t[0].y == 0 ? 0.f : INFINITY);
		t[1].y = (t[1].y == 0 ? 0.f : INFINITY);
	}

	if (transformed_ray.rd.z != 0)
	{
		t[0].z /= transformed_ray.rd.z;
		t[1].z /= transformed_ray.rd.z;
	}
	else
	{
		t[0].z = (t[0].z == 0 ? 0.f : INFINITY);
		t[1].z = (t[1].z == 0 ? 0.f : INFINITY);
	}

	// check if inside boundaries
	for (int i = 0; i < 6; ++i)
	{
		tmp = i % 3;
		if (t[i >= 3][tmp] >= 0.f)
		{
			isec_t = t[i >= 3][tmp];
			isec_p = transformed_ray.ro + isec_t * transformed_ray.rd;

			inside_1 = glm::dot(isec_p - moved_centers[i], v1[tmp]) /
				v1_dots[tmp];
			inside_2 = glm::dot(isec_p - moved_centers[i], v2[tmp]) /
				v2_dots[tmp];

			tests[i] = (0 <= inside_1) && (inside_1 <= 1)
				&& (0 <= inside_2) && (inside_2 <= 1);
		}
	}

	isec_t = INFINITY;
	for (int i = 0; i < 6; ++i)
	{
		tmp = i % 3;
		if (tests[i])
		{
			if (isec_t > t[i >= 3][tmp])
			{
				isec_t = t[i >= 3][tmp];
			}
		}
	}
	return isec_t;
}

float Triangle::intersect(const Ray &ray, SurfaceInteraction * isect)
{
	float t_plane = INFINITY;
	Plane plane{ p1, n };

	// intersect without updating nearest intersection parameter
	t_plane = plane.intersect(ray);

	glm::vec3 t_vec = m_inv * (ray.ro + t_plane * ray.rd);

	if (t_vec.y + t_vec.z <= 1 &&
		t_vec.y >= 0 &&
		t_vec.z >= 0)
	{
		if (t_plane < ray.tNearest)
		{
			ray.tNearest = t_plane;
			isect->p = ray.ro + ray.rd * t_plane;
			isect->normal = get_normal(isect->p);
			isect->mat = mat;
		}

		return t_plane;
	}
	return INFINITY;
}

float UnitCube::intersect(const Ray & ray, SurfaceInteraction * isect)
{
	assert(abs(length(ray.rd)) > 0);

	Ray transformed_ray{ world_to_obj * glm::vec4(ray.ro, 1.f),
		world_to_obj * glm::vec4(ray.rd, 0.f) };


	// no need for checking division by zero, floating point arithmetic is helping here
	glm::vec3 inv_rd = 1.f / transformed_ray.rd;
	glm::vec3 t[2] = { glm::vec3(INFINITY), glm::vec3(INFINITY) };
	// interval of intersection
	float t0 = 0.f, t1 = INFINITY;

	// the case where the ray is parallel to the plane is handled correctly by these two
	// calculations => if the ray is outside the slabs, the values will both be -/+ inf,
	// if it is inside, the values will be inf with different signs
	t[0] = -boundaries - transformed_ray.ro;
	t[1] = boundaries - transformed_ray.ro;

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

	if (t0 >= 0 && t0 < INFINITY)
	{
		if (t0 < ray.tNearest)
		{
			// update maximum intersection parameter
			ray.tNearest = t0;
			// update intersection properties
			isect->p = ray.ro + ray.rd * t0;
			isect->normal = get_normal(isect->p);
			isect->mat = mat;
		}
	}

	return t0;
}
} //namespace rt
