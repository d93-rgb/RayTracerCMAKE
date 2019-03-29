#pragma once
#include "core/renderer.h"
#include "shape/ray.h"
#include "scene/scene.h"
#include "shape/shape.h"
#include "light/light.h"

namespace rt
{
/*
	Calculate the normalized reflection vector.
	dir	: the incident ray
	N	: the normalized normal vector of a surface
*/
glm::vec3 reflect(glm::vec3 dir, glm::vec3 N)
{
	return glm::normalize(dir - 2 * glm::dot(N, dir) * N);
}

/*
	Calculate the normalized refracted vector.
	V	: the view direction
	N	: the normalized normal vector of a surface
*/
bool refract(glm::vec3 V, glm::vec3 N, float refr_idx, glm::vec3 *refracted)
{
	float cos_alpha = glm::dot(-V, N);

	// TODO: calculate refracted ray when coming from a medium other than air
	// refractive index of air = 1.f
	float eta = 1.f / refr_idx;

	if (cos_alpha < 0.f)
	{
		eta = 1.f / eta;
		cos_alpha *= -1;
		N = -N;
	}

	float radicand = 1.f - eta * eta * (1.f - cos_alpha * cos_alpha);

	// check for total internal reflection
	if (radicand < 0.f)
	{
		*refracted = glm::vec3(0.f);
		return false;
	}

	*refracted = glm::normalize(eta * V - (eta * cos_alpha + sqrt(radicand)) * N);
	return true;
}

/*
	Compute the fresnel term, that is, the factor for reflective contribution
	rel_eta: the relative refractive coefficient
	c: the cosine of the angle between incident and normal ray
*/
float fresnel(float rel_eta, float c)
{

	if (c < 0.f)
	{
		c = -c;
		rel_eta = 1.f / rel_eta;
	}
	// using Schlick's approximation
	float r0 = (rel_eta - 1.f) / (rel_eta + 1.f);
	r0 = r0 * r0;

	c = 1.f - c;

	return r0 + (1.f - r0) * powf(c, 5);
}

glm::vec3 handle_reflection(const Scene &s,
	const Ray &ray,
	const glm::vec3 &isect_p,
	SurfaceInteraction *isect,
	int depth)
{
	glm::vec3 reflected = reflect(ray.rd, isect->normal);

	return shoot_recursively(s, 
		Ray(isect_p + shadowEpsilon * reflected, reflected), 
		isect, 
		++depth);
}

glm::vec3 handle_transmission(const Scene &s,
	const Ray &ray,
	const glm::vec3 &isect_p,
	SurfaceInteraction *isect,
	int depth)
{
	glm::vec3 reflected, refracted;
	float f;

	reflected = reflect(ray.rd, isect->normal);

	// check for total internal reflection
	if (!refract(ray.rd, isect->normal, isect->mat->getRefractiveIdx(), &refracted))
	{
		//reflected = glm::normalize(reflect(ray.rd, (*o)->get_normal(isect_p)));
		return shoot_recursively(s, 
			Ray(isect_p + shadowEpsilon * reflected, reflected), 
			isect, 
			++depth);
	}

	f = fresnel(1.f / isect->mat->getRefractiveIdx(),
		glm::dot(-ray.rd, isect->normal));
	++depth;

	return f * shoot_recursively(s, 
		Ray(isect_p + shadowEpsilon * reflected, reflected), 
		isect,
		depth) +
		(1.f - f) * shoot_recursively(s, 
			Ray(isect_p + shadowEpsilon * refracted, refracted), 
			isect, 
			depth);
}

/*
	Shoot next ray and obtain the next intersection point.
	Returns the distance to the hit surface and saves hit object
	in the given pointer 'o'.
	s: the scene with its objects
	ray: the next ray to trace
	o: the object that was hit
*/
float shoot_ray(const Scene &s, const Ray &ray, SurfaceInteraction *isect)
{
	float t_int = INFINITY;
	float tmp = INFINITY;

	// get nearest intersection point
	for (auto &objs : s.get_scene())
	{
		tmp = objs->intersect(ray, isect);

		//if (tmp >= 0 && t_int > tmp)
		//{
		//	t_int = tmp;
		//	// update intersection data with the properties of the closer object
		//	//col[i] = sphs.color * glm::max(0.f, glm::dot(-rd, sphs.get_normal(inters_p)));
		//	//std::cout << col[i].x << " " << col[i].y << " " << col[i].z << std::endl;
		//}
	}
	return ray.tNearest;
}

glm::vec3 shoot_recursively(const Scene &s,
	const Ray &ray,
	SurfaceInteraction *isect,
	int depth)
{
	if (depth == MAX_DEPTH)
	{
		return glm::vec3(0);
	}

	float distance;
	glm::vec3 contribution = glm::vec3(0);
	glm::vec3 isect_p;

	distance = shoot_ray(s, ray, isect);

	// check for no intersection
	if (distance < 0 || distance == INFINITY)
	{
		return glm::vec3(0.0f);
	}

	isect_p = ray.ro + distance * ray.rd;

	//if ((glm::length((*o)->mat->ambient) > 0) || (glm::length((*o)->mat->specular) > 0))
		// accumulate all light contribution

	// map direction of normals to a color for debugging
#ifdef DEBUG_NORMALS
	return contribution = (glm::vec3(1.f) + isect->normal) * 0.5f;
#endif

	for (auto &l : s.lights)
	{
		contribution += l->phong_shade(s,
			ray/*Ray(ray.ro + shadowEpsilon * ray.rd, ray.rd)*/,
			isect_p,
			*isect);
	}

	if (glm::length(isect->mat->getReflective()) > 0)
	{
		glm::vec3 reflective = isect->mat->getReflective();
		contribution += reflective * handle_reflection(s, ray, isect_p, isect, depth);
	}

	if (glm::length(isect->mat->getTransparent()) > 0)
	{
		glm::vec3 transparent = isect->mat->getTransparent();
		contribution += transparent * handle_transmission(s, ray, isect_p, isect, depth);
	}

	return contribution;
}
}