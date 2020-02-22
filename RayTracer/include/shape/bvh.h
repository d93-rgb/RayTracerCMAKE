#pragma once
#include "core/rt.h"
#include <vector>

namespace rt
{
class BVH_Node
{
public:
	std::unique_ptr<BVH_Node> left_node;
	std::unique_ptr<BVH_Node> right_node;
	std::unique_ptr<Bounds3> box;
	std::vector<std::shared_ptr<Shape>> shapes;

	float intersect(const Ray& ray, SurfaceInteraction* isect);
};

class BVH_Tree
{
public:
	std::unique_ptr<BVH_Node> bvh_node;
	float intersect(const Ray& ray, SurfaceInteraction* isect);
};


class BVH
{
public:
	BVH(const std::vector<std::shared_ptr<Shape>>& scene_objects,
		size_t max_triangle_count = 3,
		size_t max_depth = 40);

	bool build_bvh();
	float traverse_bvh(const Ray& ray, SurfaceInteraction* isect);

private:
	bool build_bvh(BVH_Node* current_node, int depth);

	size_t MAX_TRIANGLE_COUNT;
	size_t MAX_DEPTH;
	BVH_Tree bvh_tree;
};

} // namespace rt