#include "bvh.h"
#include "shape.h"

namespace rt
{

float BVH_Node::intersect(const Ray& ray, SurfaceInteraction* isect)
{
	if (!left_node && !right_node)
	{
		float t_min = INFINITY;
		float t_tmp;
		next_node = nullptr;

		for (const auto& object : shapes)
		{
			t_tmp = object->intersect(ray, isect);
			if (t_tmp < t_min)
			{
				t_min = t_tmp;
			}
		}
		return t_min;
	}

	if(left_node)
		float t0 = left_node->box->intersect(ray);
	
	if(right_node)
		float t1 = right_node->box->intersect(ray);

	if (t0 < t1)
	{
		left_node->intersect(ray, isect);
	}
	else
	{
		right_node->intersect(ray, isect);
	}
}

float BVH_Tree::intersect(const Ray& ray, SurfaceInteraction* isect)
{
	float t_min = INFINITY;

	t_min = bvh_node->intersect(ray, isect);

	return t_min;
}

bool BVH::build_bvh(const Bounds3& current_box)
{
	// split order x, y then z, so n goes from 0 to 2
	int n = 0;
	// middle point
	float m;

	BVH_Node l_node;
	BVH_Node r_node;

	Bounds3 first_half = current_box;
	Bounds3 second_half = current_box;

	// split the box
	m = 0.5f * (current_box.boundaries[0][n] + current_box.boundaries[1][n]);
	first_half.boundaries[1][n] = m;
	second_half.boundaries[0][n] = m;

	// assign triangles to the appropriate box
	for (const auto& t : this->scene_objects)
	{
		if (first_half[0] <= t->bounding_box[0] ||
			t->bounding_box[1] <= first_half[1])
		{
			l_node.shapes.push_back(t);
		}
		if (second_half[0] <= t->bounding_box[0] ||
			t->bounding_box[1] <= second_half[1])
		{
			r_node.shapes.push_back(t);
		}
	}
}

} //namespace rt