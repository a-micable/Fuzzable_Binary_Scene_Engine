#pragma once
#include "bse/binary_scene_engine.hpp"
namespace bse::scene {
bool hasAncestor(const Scene& scene, ObjectId node, ObjectId ancestor);
std::vector<ObjectId> descendantsOf(const Scene& scene, ObjectId root);
std::size_t depthOf(const Scene& scene, ObjectId node);
}
