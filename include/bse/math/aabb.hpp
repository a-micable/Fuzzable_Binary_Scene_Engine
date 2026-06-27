#pragma once
#include "bse/binary_scene_engine.hpp"
namespace bse::math {
struct Aabb { Vec3 min; Vec3 max; bool empty = true; };
Aabb emptyAabb();
Aabb includePoint(Aabb box, const Vec3& point);
Aabb includeMesh(const Mesh& mesh);
Vec3 center(const Aabb& box);
Vec3 extent(const Aabb& box);
bool contains(const Aabb& box, const Vec3& point);
}
