#include "shape/bvh.h"
#include "shape/shape.h"
#include "shape/ray.h"
#include "interaction/interaction.h"

namespace rt
{
float BVH_Node::intersect(const Ray& ray, SurfaceInteraction* isect)
{
	if (!left_node && !right_node)
	{
		float t_min = INFINITY;
		float t_tmp;

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
	float t0 = INFINITY;
	float t1 = INFINITY;

	if(left_node)
		t0 = left_node->box->intersect(ray);
	
	if(right_node)
		t1 = right_node->box->intersect(ray);

	if (t0 == INFINITY && t1 == INFINITY)
	{
		return INFINITY;
	}
	
	//float t0_left = INFINITY;
	//float t1_right = INFINITY;
	if (t0 < INFINITY)
	{
		t0= left_node->intersect(ray, isect);
	}
	if (t1 < INFINITY)
	{
		t1= right_node->intersect(ray, isect);
	}
	return std::min(t0, t1);
}

float BVH_Tree::intersect(const Ray& ray, SurfaceInteraction* isect)
{
	float t_min = INFINITY;

	t_min = bvh_node->intersect(ray, isect);

	return t_min;
}

// split order x, y then z, so n goes from 0 to 2
bool BVH::build_bvh(BVH_Node* current_node, int depth)
{
	if (depth > MAX_DEPTH)
	{
		return false;
	}

	// middle point
	float m;
	int n = depth % 3;

	current_node->left_node.reset(new BVH_Node());
	current_node->right_node.reset(new BVH_Node());

	current_node->left_node->box = std::make_unique<Bounds3>(current_node->box->boundaries[0],
		current_node->box->boundaries[1]);
	current_node->right_node->box = std::make_unique<Bounds3>(current_node->box->boundaries[0],
		current_node->box->boundaries[1]);

	// split the box

	// the split axis which divides the triangles into two groups
	m = 0.5f * (current_node->box->boundaries[0][n] + current_node->box->boundaries[1][n]);
	
	glm::vec3 left_min_bound(INFINITY);
	glm::vec3 left_max_bound(-INFINITY);
	glm::vec3 right_min_bound(INFINITY);
	glm::vec3 right_max_bound(-INFINITY);

	for (const auto& triangle : current_node->shapes)
	{
		if (triangle->bounding_box->centroid[n] < m)
		{
			current_node->left_node->shapes.push_back(triangle);
			left_min_bound = glm::min(left_min_bound, triangle->bounding_box->boundaries[0]);
			left_max_bound = glm::max(left_max_bound, triangle->bounding_box->boundaries[1]);
		}
		else
		{
			current_node->right_node->shapes.push_back(triangle);
			right_min_bound = glm::min(right_min_bound, triangle->bounding_box->boundaries[0]);
			right_max_bound = glm::max(right_max_bound, triangle->bounding_box->boundaries[1]);
		}
	}

	// create new box boundaries
	current_node->left_node->box->boundaries[0] = left_min_bound;
	current_node->left_node->box->boundaries[1] = left_max_bound;
	current_node->right_node->box->boundaries[0] = right_min_bound;
	current_node->right_node->box->boundaries[1] = right_max_bound;

	if (current_node->left_node->shapes.size() > MAX_TRIANGLE_COUNT)
	{
		build_bvh(current_node->left_node.get(),  depth+1);
	} 
	if (current_node->right_node->shapes.size() > MAX_TRIANGLE_COUNT)
	{
		build_bvh(current_node->right_node.get(), depth+1);
	}

	return true;
}

bool BVH::build_bvh()
{
	this->build_bvh(this->bvh_tree.bvh_node.get(), 0);
	return true;
}

float BVH::traverse_bvh(const Ray& ray, SurfaceInteraction *isect)
{
	return this->bvh_tree.intersect(ray, isect);
}

} //namespace rt