#include "bse/scene_metrics.hpp"

#include "bse/scene_stats.hpp"

#include <algorithm>
#include <cmath>
#include <sstream>
#include <unordered_map>
#include <unordered_set>

namespace bse {

namespace {

Vec3 Sub(Vec3 left, Vec3 right) {
  return {left.x - right.x, left.y - right.y, left.z - right.z};
}

Vec3 Cross(Vec3 left, Vec3 right) {
  return {left.y * right.z - left.z * right.y, left.z * right.x - left.x * right.z,
          left.x * right.y - left.y * right.x};
}

float Length(Vec3 value) {
  return std::sqrt(value.x * value.x + value.y * value.y + value.z * value.z);
}

float TriangleArea(const Vertex& a, const Vertex& b, const Vertex& c) {
  return Length(Cross(Sub(b.position, a.position), Sub(c.position, a.position))) * 0.5F;
}

float SurfaceAreaEstimate(const Mesh& mesh) {
  float area = 0.0F;
  if (!mesh.indices.empty()) {
    for (std::size_t i = 0; i + 2U < mesh.indices.size(); i += 3U) {
      const std::uint32_t ia = mesh.indices[i];
      const std::uint32_t ib = mesh.indices[i + 1U];
      const std::uint32_t ic = mesh.indices[i + 2U];
      if (ia < mesh.vertices.size() && ib < mesh.vertices.size() && ic < mesh.vertices.size()) {
        area += TriangleArea(mesh.vertices[ia], mesh.vertices[ib], mesh.vertices[ic]);
      }
    }
  } else {
    for (std::size_t i = 0; i + 2U < mesh.vertices.size(); i += 3U) {
      area += TriangleArea(mesh.vertices[i], mesh.vertices[i + 1U], mesh.vertices[i + 2U]);
    }
  }
  return area;
}

std::size_t TriangleCount(const Mesh& mesh) {
  if (!mesh.indices.empty()) return mesh.indices.size() / 3U;
  return mesh.vertices.size() / 3U;
}

MeshComplexity MeasureMesh(const Mesh& mesh) {
  MeshComplexity measured;
  measured.id = mesh.id;
  measured.name = mesh.name;
  measured.vertices = mesh.vertices.size();
  measured.indices = mesh.indices.size();
  measured.triangles = TriangleCount(mesh);
  measured.surface_area_estimate = SurfaceAreaEstimate(mesh);
  return measured;
}

std::string BucketLabel(std::size_t low, std::size_t high) {
  std::ostringstream out;
  if (low == high) {
    out << low;
  } else {
    out << low << "-" << high;
  }
  return out.str();
}

std::vector<HistogramBucket> Bucketize(const std::vector<std::size_t>& values,
                                       const std::vector<std::size_t>& cutoffs) {
  std::vector<HistogramBucket> buckets;
  if (cutoffs.empty()) return buckets;
  buckets.reserve(cutoffs.size());
  std::size_t low = 0;
  for (std::size_t cutoff : cutoffs) {
    buckets.push_back({BucketLabel(low, cutoff), 0});
    low = cutoff + 1U;
  }
  buckets.push_back({std::to_string(low) + "+", 0});
  for (std::size_t value : values) {
    bool placed = false;
    for (std::size_t i = 0; i < cutoffs.size(); ++i) {
      if (value <= cutoffs[i]) {
        ++buckets[i].count;
        placed = true;
        break;
      }
    }
    if (!placed) ++buckets.back().count;
  }
  return buckets;
}

std::unordered_map<ObjectId, const Node*> BuildNodeMap(const Scene& scene) {
  std::unordered_map<ObjectId, const Node*> nodes;
  for (const auto& node : scene.nodes) nodes[node.id] = &node;
  return nodes;
}

std::size_t NodeDepth(const Node& node, const std::unordered_map<ObjectId, const Node*>& nodes) {
  std::unordered_set<ObjectId> visited;
  std::size_t depth = 0;
  ObjectId parent = node.parent;
  while (parent != kInvalidObjectId) {
    auto iter = nodes.find(parent);
    if (iter == nodes.end()) break;
    if (!visited.insert(iter->second->id).second) break;
    parent = iter->second->parent;
    ++depth;
  }
  return depth;
}

HierarchyMetrics MeasureHierarchy(const Scene& scene) {
  HierarchyMetrics metrics;
  const auto nodes = BuildNodeMap(scene);
  std::vector<std::size_t> depths;
  std::vector<std::size_t> branch_counts;
  std::size_t total_children = 0;
  for (const auto& node : scene.nodes) {
    if (node.parent == kInvalidObjectId) ++metrics.roots;
    if (node.children.empty()) ++metrics.leaves;
    const std::size_t depth = NodeDepth(node, nodes);
    metrics.max_depth = std::max(metrics.max_depth, depth);
    depths.push_back(depth);
    branch_counts.push_back(node.children.size());
    total_children += node.children.size();
  }
  if (!scene.nodes.empty()) {
    metrics.average_children = static_cast<float>(total_children) / static_cast<float>(scene.nodes.size());
  }
  metrics.depth_histogram = Bucketize(depths, {0, 1, 2, 4, 8, 16, 32, 64});
  metrics.branching_histogram = Bucketize(branch_counts, {0, 1, 2, 4, 8, 16, 32});
  return metrics;
}

ResourceMetrics MeasureResources(const Scene& scene) {
  ResourceMetrics metrics;
  for (const auto& texture : scene.textures) {
    metrics.total_texture_pixels += static_cast<std::size_t>(texture.width) *
                                    static_cast<std::size_t>(texture.height);
    if (texture.uri.find("data:") == 0) {
      ++metrics.textures_with_embedded_uri;
    } else if (!texture.uri.empty()) {
      ++metrics.textures_with_external_uri;
    }
  }
  const auto referenced_materials = FindReferencedMaterials(scene);
  const auto referenced_textures = FindReferencedTextures(scene);
  std::unordered_set<ObjectId> material_ids(referenced_materials.begin(), referenced_materials.end());
  std::unordered_set<ObjectId> texture_ids(referenced_textures.begin(), referenced_textures.end());
  for (const auto& material : scene.materials) {
    if (material_ids.find(material.id) == material_ids.end()) ++metrics.unreferenced_materials;
  }
  for (const auto& texture : scene.textures) {
    if (texture_ids.find(texture.id) == texture_ids.end()) ++metrics.unreferenced_textures;
  }
  return metrics;
}

std::vector<HistogramBucket> MeasureAnimationKeys(const Scene& scene) {
  std::vector<std::size_t> values;
  for (const auto& animation : scene.animations) {
    for (const auto& channel : animation.channels) values.push_back(channel.keys.size());
  }
  return Bucketize(values, {0, 1, 2, 4, 8, 16, 32, 64, 128});
}

float ScoreComplexity(const SceneComplexityMetrics& metrics) {
  float score = 0.0F;
  score += static_cast<float>(metrics.hierarchy.max_depth) * 2.0F;
  score += metrics.hierarchy.average_children * 8.0F;
  score += static_cast<float>(metrics.resources.total_texture_pixels) / 1'000'000.0F;
  score += static_cast<float>(metrics.resources.unreferenced_materials + metrics.resources.unreferenced_textures) * 0.5F;
  for (const auto& mesh : metrics.meshes) {
    score += static_cast<float>(mesh.vertices) * 0.002F;
    score += static_cast<float>(mesh.triangles) * 0.004F;
    score += mesh.surface_area_estimate * 0.001F;
  }
  for (const auto& bucket : metrics.animation_key_histogram) {
    score += static_cast<float>(bucket.count) * 0.25F;
  }
  return score;
}

void WriteHistogram(std::ostringstream* out, const std::string& name,
                    const std::vector<HistogramBucket>& buckets) {
  *out << name << "\n";
  for (const auto& bucket : buckets) {
    *out << "  " << bucket.label << ": " << bucket.count << "\n";
  }
}

}  // namespace

SceneComplexityMetrics ComputeSceneComplexity(const Scene& scene) {
  SceneComplexityMetrics metrics;
  metrics.scene_name = scene.name;
  metrics.hierarchy = MeasureHierarchy(scene);
  metrics.resources = MeasureResources(scene);
  metrics.meshes = RankMeshesByComplexity(scene);
  metrics.animation_key_histogram = MeasureAnimationKeys(scene);
  metrics.complexity_score = ScoreComplexity(metrics);
  return metrics;
}

std::vector<MeshComplexity> RankMeshesByComplexity(const Scene& scene) {
  std::vector<MeshComplexity> meshes;
  meshes.reserve(scene.meshes.size());
  for (const auto& mesh : scene.meshes) meshes.push_back(MeasureMesh(mesh));
  std::sort(meshes.begin(), meshes.end(), [](const MeshComplexity& left, const MeshComplexity& right) {
    if (left.triangles != right.triangles) return left.triangles > right.triangles;
    if (left.vertices != right.vertices) return left.vertices > right.vertices;
    return left.id < right.id;
  });
  return meshes;
}

std::string FormatSceneComplexity(const SceneComplexityMetrics& metrics) {
  std::ostringstream out;
  out << "scene=" << metrics.scene_name << "\n";
  out << "score=" << metrics.complexity_score << "\n";
  out << "roots=" << metrics.hierarchy.roots << "\n";
  out << "leaves=" << metrics.hierarchy.leaves << "\n";
  out << "max_depth=" << metrics.hierarchy.max_depth << "\n";
  out << "average_children=" << metrics.hierarchy.average_children << "\n";
  out << "texture_pixels=" << metrics.resources.total_texture_pixels << "\n";
  out << "unreferenced_materials=" << metrics.resources.unreferenced_materials << "\n";
  out << "unreferenced_textures=" << metrics.resources.unreferenced_textures << "\n";
  WriteHistogram(&out, "depths", metrics.hierarchy.depth_histogram);
  WriteHistogram(&out, "branching", metrics.hierarchy.branching_histogram);
  WriteHistogram(&out, "animation_keys", metrics.animation_key_histogram);
  out << "meshes\n";
  for (const auto& mesh : metrics.meshes) {
    out << "  " << mesh.id << " " << mesh.name << " vertices=" << mesh.vertices
        << " indices=" << mesh.indices << " triangles=" << mesh.triangles
        << " area=" << mesh.surface_area_estimate << "\n";
  }
  return out.str();
}

}  // namespace bse
