#include "shape/quadric/quadrics.h"

namespace rt
{

// TODO: implement function for solving a quadratic equation
bool Quadric::solveQuadraticEq(double* t, double a, double b, double c)
{
	double disc = sqrt(b * b - 4 * a * c);

	if (disc < 0.0f)
	{
		return false;
	}

	double t0;
	double t1;
	double q;

	// numerically unstable, if difference |b-disc| is small
	//double t0 = (-b + disc) / a * 0.5;
	//double t1 = (-b - disc) / a * 0.5;

	if (b < 0)
	{
		q = -.5f * (b - disc);
	}
	else
	{
		q = -.5f * (b + disc);
	}
	
	t0 = q / a;
	t1 = c / q;

	if (t0 > t1)
	{
		std::swap(t0, t1);
	}
	*t = t1 < 0.0 ? INFINITY : (t0 < 0.0 ? t1 : t0);
	return *t < INFINITY;
}

double Sphere::intersect(const Ray& ray, SurfaceInteraction* isect)
{
	double t1 = INFINITY, t2 = t1;
	double tmp;

	double term_1 = glm::dot(ray.rd, ray.rd);
	double term_2 = 2 * glm::dot(ray.rd, ray.ro - origin);
	double term_3 = glm::dot(ray.ro - origin, ray.ro - origin) - r * r;
	/*
	double disc = term_2 * term_2 - 4 * term_1 * term_3;

	if (disc < 0)
	{
		return INFINITY;
	}

	tmp = sqrt(disc);
	t1 = (-term_2 + tmp) / term_1 * 0.5f;
	t2 = (-term_2 - tmp) / term_1 * 0.5f;

	tmp = std::fmin(t1, t2);
	tmp = tmp >= 0 ? tmp : fmax(t1, t2);
	tmp = tmp >= 0 ? tmp : INFINITY;
	*/
	if (Quadric::solveQuadraticEq(&tmp, term_1, term_2, term_3))
	{
		if (tmp < ray.tNearest)
		{
			ray.tNearest = tmp;
			isect->p = ray.ro + ray.rd * tmp;
			isect->normal = get_normal(isect->p);
			isect->mat = mat;
		}
	}
	return tmp;
}

double Cylinder::intersect(const Ray& ray, SurfaceInteraction* isect)
{
	Ray transformed_ray{ worldToObj * glm::dvec4(ray.ro, 1.f),
		worldToObj * glm::dvec4(ray.rd, 0.f) };
	glm::dvec2 t_ro = glm::dvec2(transformed_ray.ro.x, transformed_ray.ro.z);
	glm::dvec2 t_rd = glm::dvec2(transformed_ray.rd.x, transformed_ray.rd.z);

	glm::dvec3 isect_p1 = glm::dvec3(INFINITY), isect_p2 = glm::dvec3(INFINITY);
	double x1, x2, tmp1 = INFINITY, tmp2 = INFINITY;
	double a = glm::dot(t_rd, t_rd);
	double b = 2 * glm::dot(t_ro, t_rd);
	double c = glm::dot(t_ro, t_ro) - radius * radius;
	int surf_hit = 0;

	double discr = b * b - 4 * a * c;

	if (discr < 0)
	{
		return INFINITY;
	}

	discr = sqrt(discr);
	x1 = (-b + discr) / a * 0.5f;
	x2 = (-b - discr) / a * 0.5f;

	// get intersection points
	if (x1 > 0)
	{
		isect_p1 = transformed_ray.ro + x1 * transformed_ray.rd;
	}

	if (x2 > 0)
	{
		isect_p2 = transformed_ray.ro + x2 * transformed_ray.rd;
	}

	// no intersection found check
	if ((isect_p1 == glm::dvec3(INFINITY)) && (isect_p2 == glm::dvec3(INFINITY)))
	{
		return INFINITY;
	}

	// check if inside the given height boundary
	if (isect_p1.y >= 0 && isect_p1.y <= height)
	{
		tmp1 = x1;
		++surf_hit;
	}

	if (isect_p2.y >= 0 && isect_p2.y <= height)
	{
		tmp2 = x2;
		++surf_hit;
	}

	tmp2 = std::min(tmp1, tmp2);

	if (tmp2 < ray.tNearest)
	{
		ray.tNearest = tmp2;
		isect->p = ray.ro + ray.rd * tmp2;
		isect->normal = get_normal(transformed_ray.ro + tmp2 * transformed_ray.rd, surf_hit);
		isect->mat = mat;
		isect->texture = nullptr;
	}

	return tmp2;
}


}