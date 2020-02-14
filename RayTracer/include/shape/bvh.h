#pragma once
#include "rt.h"
#include <vector>

namespace rt
{
typedef struct BVH_Node
{
	BVH_Node* left_node;
	BVH_Node* right_node;
	Bounds3* box;
	std::vector<Shape*> shapes;

	float intersect(const Ray& ray, SurfaceInteraction* isect);
} BVH_Node;

typedef struct BVH_Tree
{
	BVH_Node* bvh_node;
	float intersect(const Ray& ray, SurfaceInteraction* isect);

} BVH_Tree;


class BVH
{
public:
	BVH(const std::vector<Shape*>& scene_objects, 
		const Bounds3& surrounding_box, 
		size_t max_triangle_count = 5) :
		scene_objects(scene_objects), 
		surrounding_box(surrounding_box)
		MAX_TRIANGLE_COUNT(max_triangle_count)
	{}

	bool build_bvh(const Bounds3& current_box);
	bool traverse_bvh();

	float intersect(const Ray& ray, SurfaceInteraction* isect);

private:
	size_t MAX_TRIANGLE_COUNT;
	std::vector<Shape*> scene_objects;
	Bounds3 surrounding_box;
};

} // namespace rt