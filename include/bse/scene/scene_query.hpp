#pragma once
#include "bse/binary_scene_engine.hpp"
namespace bse::scene {
std::vector<const SceneNode*> nodesUsingMesh(const Scene& scene, ObjectId meshId);
std::vector<const Mesh*> meshesUsingMaterial(const Scene& scene, ObjectId materialId);
const SceneNode* firstCameraNode(const Scene& scene);
}
