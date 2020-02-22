#include "shape/shape.h"
#include "shape/bvh.h"

namespace rt
{
double Plane::intersect(const Ray& ray, SurfaceInteraction* isect)
{
	double denom = glm::dot(normal, ray.rd);

	if (abs(denom) < 1e-6) return INFINITY;

	double num = k - glm::dot(normal, ray.ro);

	double t = num / denom;

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

double Plane::intersect(const Ray& ray)
{
	double denom = glm::dot(normal, ray.rd);

	if (abs(denom) < 1e-6) return INFINITY;

	double num = k - glm::dot(normal, ray.ro);

	double t = num / denom;

	return t >= 0 ? t : INFINITY;
}

double Rectangle::intersect(const Ray& ray, SurfaceInteraction* isect)
{
	double denom = glm::dot(ray.rd, normal);

	if (abs(denom) < 1e-6) return INFINITY;

	double num = glm::dot(normal, center - ray.ro);

	double t = num / denom;

	if (t < 0) return INFINITY;

	glm::dvec3 isec_p = ray.ro + t * ray.rd;

	double inside_1 = glm::dot(isec_p - center, v1) / v1_dot;
	double inside_2 = glm::dot(isec_p - center, v2) / v2_dot;

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

double Cube::intersect(const Ray& ray, SurfaceInteraction* isect)
{
	assert(abs(length(ray.rd)) > 0);

	Ray transformed_ray{ world_to_obj * glm::dvec4(ray.ro, 1.f),
		world_to_obj * glm::dvec4(ray.rd, 0.f) };

	int nearest = 0;
	int tmp = 0;
	double isec_t = INFINITY;
	double inside_1;
	double inside_2;
	bool tests[6] = { false };
	glm::dvec3 t[2] = { glm::dvec3(INFINITY), glm::dvec3(INFINITY) };
	glm::dvec3 isec_p;


	// calculate the 6 transformed_ray-plane intersections
	t[0] = (boundaries - transformed_ray.ro);
	t[1] = -(boundaries + transformed_ray.ro);

	// not regarding IEEE doubleing point arithmetics here, see Bounds3 for a better variant
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

double Cube::intersect(const Ray& ray)
{
	assert(abs(length(ray.rd)) > 0);

	Ray transformed_ray{ world_to_obj * glm::dvec4(ray.ro, 1.f),
		world_to_obj * glm::dvec4(ray.rd, 0.f) };

	int nearest = 0;
	int tmp = 0;
	double isec_t = INFINITY;
	double inside_1;
	double inside_2;
	bool tests[6] = { false };
	glm::dvec3 t[2] = { glm::dvec3(INFINITY), glm::dvec3(INFINITY) };
	glm::dvec3 isec_p;


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

//double Triangle::intersect(const Ray& ray, SurfaceInteraction* isect)
//{
//	double t_plane = INFINITY;
//	Plane plane{ p1, plane_normal };
//
//	// intersect without updating nearest intersection parameter
//	t_plane = plane.intersect(ray);
//	if(t_plane == INFINITY)
//	{
//		return INFINITY;
//	}
//	glm::dvec3 p_isect = (ray.ro + t_plane * ray.rd);
//
//	//glm::dvec3 t_vec = m_inv * p_isect;
//	glm::dvec2 t_vec;
//	t_vec.x = p1.y*p3.x - p1.x*p3.y + (p3.y - p1.y)*p_isect.x + (p1.x - p3.x)*p_isect.y;
//	t_vec.y = p1.x * p2.y - p1.y * p2.x + (p1.y - p2.y) * p_isect.x + (p2.x - p1.x) * p_isect.y;
//
//	t_vec /= (-p2.y * p3.x + p1.y * (-p2.x + p3.x) + p1.x * (p2.y - p3.y) + p2.x * p3.y);
//
//	if (((t_vec.y + t_vec.x) <= 1.0f) &&
//		(t_vec.x >= 0) &&
//		(t_vec.y >= 0))
//	{
//		if (t_plane < ray.tNearest)
//		{
//			ray.tNearest = t_plane;
//			isect->p = p_isect;
//			isect->normal = get_normal(isect->p);
//			isect->mat = mat;
//		}
//
//		return t_plane;
//	}
//	return INFINITY;
//}

int MaxDimension(const glm::dvec3& v) {
	return (v.x > v.y) ? ((v.x > v.z) ? 0 : 2) : ((v.y > v.z) ? 1 : 2);
}

glm::dvec3 Permute(const glm::dvec3& v, int x, int y, int z) {
	return glm::dvec3(v[x], v[y], v[z]);
}

double Triangle::intersect(const Ray& ray, SurfaceInteraction* isect)
{
	// Get triangle vertices in _p0_, _p1_, and _p2_

	// Perform ray--triangle intersection test

	// Transform triangle vertices to ray coordinate space

	// Translate vertices based on ray origin
	glm::dvec3 p0t = p1 - ray.ro;
	glm::dvec3 p1t = p2 - ray.ro;
	glm::dvec3 p2t = p3 - ray.ro;

	// Permute components of triangle vertices and ray direction
	int kz = MaxDimension(glm::abs(ray.rd));
	int kx = kz + 1;
	if (kx == 3) kx = 0;
	int ky = kx + 1;
	if (ky == 3) ky = 0;
	glm::dvec3 d = Permute(ray.rd, kx, ky, kz);
	p0t = Permute(p0t, kx, ky, kz);
	p1t = Permute(p1t, kx, ky, kz);
	p2t = Permute(p2t, kx, ky, kz);

	// Apply shear transformation to translated vertex positions
	double Sx = -d.x / d.z;
	double Sy = -d.y / d.z;
	double Sz = 1.f / d.z;
	p0t.x += Sx * p0t.z;
	p0t.y += Sy * p0t.z;
	p1t.x += Sx * p1t.z;
	p1t.y += Sy * p1t.z;
	p2t.x += Sx * p2t.z;
	p2t.y += Sy * p2t.z;

	// Compute edge function coefficients _e0_, _e1_, and _e2_
	double e0 = p1t.x * p2t.y - p1t.y * p2t.x;
	double e1 = p2t.x * p0t.y - p2t.y * p0t.x;
	double e2 = p0t.x * p1t.y - p0t.y * p1t.x;

	// Fall back to double precision test at triangle edges
	if ((e0 == 0.0f || e1 == 0.0f || e2 == 0.0f)) {
		double p2txp1ty = (double)p2t.x * (double)p1t.y;
		double p2typ1tx = (double)p2t.y * (double)p1t.x;
		e0 = (double)(p2typ1tx - p2txp1ty);
		double p0txp2ty = (double)p0t.x * (double)p2t.y;
		double p0typ2tx = (double)p0t.y * (double)p2t.x;
		e1 = (double)(p0typ2tx - p0txp2ty);
		double p1txp0ty = (double)p1t.x * (double)p0t.y;
		double p1typ0tx = (double)p1t.y * (double)p0t.x;
		e2 = (double)(p1typ0tx - p1txp0ty);
	}

	// Perform triangle edge and determinant tests
	if ((e0 < 0 || e1 < 0 || e2 < 0) && (e0 > 0 || e1 > 0 || e2 > 0))
		return INFINITY;
	double det = e0 + e1 + e2;
	if (det == 0) return INFINITY;

	// Compute scaled hit distance to triangle and test against ray $t$ range
	p0t.z *= Sz;
	p1t.z *= Sz;
	p2t.z *= Sz;
	double tScaled = e0 * p0t.z + e1 * p1t.z + e2 * p2t.z;
	if (det < 0 && (tScaled >= 0 || tScaled < ray.tNearest * det))
		return INFINITY;
	else if (det > 0 && (tScaled <= 0 || tScaled > ray.tNearest* det))
		return INFINITY;

	double invDet = 1 / det;
	double t = tScaled * invDet;

	if (t < ray.tNearest)
	{
		ray.tNearest = t;
		isect->p = ray.ro + t * ray.rd;
		isect->normal = get_normal(isect->p);
		isect->mat = mat;
	}
	return t;
}

double UnitCube::intersect(const Ray& ray, SurfaceInteraction* isect)
{
	assert(abs(length(ray.rd)) > 0);

	Ray transformed_ray{ world_to_obj * glm::dvec4(ray.ro, 1.0),
		world_to_obj * glm::dvec4(ray.rd, 0.0) };


	// no need for checking division by zero, doubleing point arithmetic is helping here
	glm::dvec3 inv_rd = 1.0 / transformed_ray.rd;
	glm::dvec3 t[2] = { glm::dvec3(INFINITY), glm::dvec3(INFINITY) };
	// interval of intersection
	double t0 = 0.0, t1 = INFINITY;

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

double TriangleMesh::intersect(const Ray& ray, SurfaceInteraction* isect)
{
	double t_int = INFINITY;
	double tmp = INFINITY;
	bool hit = false;

	return bvh->traverse_bvh(ray, isect);
	/*hit = boundary->intersect(ray);
	if (!hit) {
		return INFINITY;
	}*/

	// get nearest intersection point
	/*for (auto &objs : tr_mesh)
	{
		tmp = objs->intersect(ray, isect);

		if (tmp >= 0 && t_int > tmp)
		{
			t_int = tmp;
		}
	}
	return t_int;*/
}

} //namespace rt
