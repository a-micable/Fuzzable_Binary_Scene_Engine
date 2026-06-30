#pragma once

#include "bse/scene.hpp"
#include "bse/status.hpp"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace bse {

struct GeometryStats {
  std::size_t mesh_count = 0;
  std::size_t vertex_count = 0;
  std::size_t index_count = 0;
  std::size_t triangle_count = 0;
  std::size_t unindexed_mesh_count = 0;
};

struct ResourceStats {
  std::size_t texture_count = 0;
  std::size_t material_count = 0;
  std::size_t referenced_texture_count = 0;
  std::size_t referenced_material_count = 0;
};

struct AnimationStats {
  std::size_t animation_count = 0;
  std::size_t channel_count = 0;
  std::size_t key_count = 0;
  float longest_duration_seconds = 0.0F;
};

struct SceneStats {
  std::size_t node_count = 0;
  std::size_t root_node_count = 0;
  std::size_t leaf_node_count = 0;
  std::size_t camera_count = 0;
  std::size_t light_count = 0;
  std::size_t skeleton_count = 0;
  std::size_t skeleton_joint_count = 0;
  GeometryStats geometry;
  ResourceStats resources;
  AnimationStats animation;
};

struct Bounds3 {
  Vec3 min;
  Vec3 max;
  bool valid = false;
};

struct SceneIssue {
  std::string code;
  std::string subject;
  std::string message;
};

SceneStats ComputeSceneStats(const Scene& scene);
Bounds3 ComputeMeshBounds(const Mesh& mesh);
Bounds3 ComputeSceneBounds(const Scene& scene);
std::vector<ObjectId> FindRootNodes(const Scene& scene);
std::vector<ObjectId> FindLeafNodes(const Scene& scene);
std::vector<ObjectId> FindReachableNodes(const Scene& scene);
std::vector<ObjectId> FindUnreachableNodes(const Scene& scene);
std::vector<ObjectId> FindReferencedMaterials(const Scene& scene);
std::vector<ObjectId> FindReferencedTextures(const Scene& scene);
std::vector<SceneIssue> AuditSceneStructure(const Scene& scene);
std::string FormatSceneStats(const SceneStats& stats);

}  // namespace bse
