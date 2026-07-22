#pragma once

#include "bse/scene.hpp"

#include <cstddef>
#include <string>
#include <vector>

namespace bse {

struct HistogramBucket {
  std::string label;
  std::size_t count = 0;
};

struct MeshComplexity {
  ObjectId id = kInvalidObjectId;
  std::string name;
  std::size_t vertices = 0;
  std::size_t indices = 0;
  std::size_t triangles = 0;
  float surface_area_estimate = 0.0F;
};

struct HierarchyMetrics {
  std::size_t roots = 0;
  std::size_t leaves = 0;
  std::size_t max_depth = 0;
  float average_children = 0.0F;
  std::vector<HistogramBucket> depth_histogram;
  std::vector<HistogramBucket> branching_histogram;
};

struct ResourceMetrics {
  std::size_t textures_with_external_uri = 0;
  std::size_t textures_with_embedded_uri = 0;
  std::size_t total_texture_pixels = 0;
  std::size_t unreferenced_materials = 0;
  std::size_t unreferenced_textures = 0;
};

struct SceneComplexityMetrics {
  std::string scene_name;
  HierarchyMetrics hierarchy;
  ResourceMetrics resources;
  std::vector<MeshComplexity> meshes;
  std::vector<HistogramBucket> animation_key_histogram;
  float complexity_score = 0.0F;
};

SceneComplexityMetrics ComputeSceneComplexity(const Scene& scene);
std::vector<MeshComplexity> RankMeshesByComplexity(const Scene& scene);
std::string FormatSceneComplexity(const SceneComplexityMetrics& metrics);

}  // namespace bse
