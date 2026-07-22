#include "bse/scene_stats.hpp"

#include <algorithm>
#include <cmath>
#include <deque>
#include <sstream>
#include <unordered_map>
#include <unordered_set>

namespace bse {

namespace {

void IncludePoint(Bounds3* bounds, const Vec3& point) {
  if (!bounds->valid) {
    bounds->min = point;
    bounds->max = point;
    bounds->valid = true;
    return;
  }
  bounds->min.x = std::min(bounds->min.x, point.x);
  bounds->min.y = std::min(bounds->min.y, point.y);
  bounds->min.z = std::min(bounds->min.z, point.z);
  bounds->max.x = std::max(bounds->max.x, point.x);
  bounds->max.y = std::max(bounds->max.y, point.y);
  bounds->max.z = std::max(bounds->max.z, point.z);
}

void IncludeBounds(Bounds3* target, const Bounds3& source) {
  if (!source.valid) {
    return;
  }
  IncludePoint(target, source.min);
  IncludePoint(target, source.max);
}

std::unordered_map<ObjectId, const Node*> NodeIndex(const Scene& scene) {
  std::unordered_map<ObjectId, const Node*> nodes;
  nodes.reserve(scene.nodes.size());
  for (const auto& node : scene.nodes) {
    nodes.emplace(node.id, &node);
  }
  return nodes;
}

std::unordered_map<ObjectId, const Mesh*> MeshIndex(const Scene& scene) {
  std::unordered_map<ObjectId, const Mesh*> meshes;
  meshes.reserve(scene.meshes.size());
  for (const auto& mesh : scene.meshes) {
    meshes.emplace(mesh.id, &mesh);
  }
  return meshes;
}

bool ContainsId(const std::unordered_set<ObjectId>& ids, ObjectId id) {
  return id != kInvalidObjectId && ids.find(id) != ids.end();
}

std::vector<ObjectId> SortedIds(std::unordered_set<ObjectId> ids) {
  std::vector<ObjectId> out(ids.begin(), ids.end());
  std::sort(out.begin(), out.end());
  return out;
}

std::string FormatFloat(float value) {
  std::ostringstream out;
  out.setf(std::ios::fixed);
  out.precision(3);
  out << value;
  return out.str();
}

}  // namespace

SceneStats ComputeSceneStats(const Scene& scene) {
  SceneStats stats;
  stats.node_count = scene.nodes.size();
  stats.camera_count = scene.cameras.size();
  stats.light_count = scene.lights.size();
  stats.skeleton_count = scene.skeletons.size();
  stats.geometry.mesh_count = scene.meshes.size();
  stats.resources.texture_count = scene.textures.size();
  stats.resources.material_count = scene.materials.size();
  stats.animation.animation_count = scene.animations.size();

  for (const auto& node : scene.nodes) {
    if (node.parent == kInvalidObjectId) {
      ++stats.root_node_count;
    }
    if (node.children.empty()) {
      ++stats.leaf_node_count;
    }
  }

  std::unordered_set<ObjectId> referenced_materials;
  for (const auto& mesh : scene.meshes) {
    stats.geometry.vertex_count += mesh.vertices.size();
    stats.geometry.index_count += mesh.indices.size();
    if (mesh.indices.empty()) {
      ++stats.geometry.unindexed_mesh_count;
      stats.geometry.triangle_count += mesh.vertices.size() / 3U;
    } else {
      stats.geometry.triangle_count += mesh.indices.size() / 3U;
    }
    if (mesh.material != kInvalidObjectId) {
      referenced_materials.insert(mesh.material);
    }
  }
  stats.resources.referenced_material_count = referenced_materials.size();

  std::unordered_set<ObjectId> referenced_textures;
  for (const auto& material : scene.materials) {
    if (material.base_color_texture != kInvalidObjectId) {
      referenced_textures.insert(material.base_color_texture);
    }
  }
  stats.resources.referenced_texture_count = referenced_textures.size();

  for (const auto& skeleton : scene.skeletons) {
    stats.skeleton_joint_count += skeleton.joints.size();
  }

  for (const auto& animation : scene.animations) {
    stats.animation.longest_duration_seconds =
        std::max(stats.animation.longest_duration_seconds, animation.duration_seconds);
    stats.animation.channel_count += animation.channels.size();
    for (const auto& channel : animation.channels) {
      stats.animation.key_count += channel.keys.size();
    }
  }

  return stats;
}

Bounds3 ComputeMeshBounds(const Mesh& mesh) {
  Bounds3 bounds;
  for (const auto& vertex : mesh.vertices) {
    IncludePoint(&bounds, vertex.position);
  }
  return bounds;
}

Bounds3 ComputeSceneBounds(const Scene& scene) {
  Bounds3 bounds;
  const auto meshes = MeshIndex(scene);
  for (const auto& node : scene.nodes) {
    auto mesh_it = meshes.find(node.mesh);
    if (mesh_it == meshes.end()) {
      continue;
    }
    Bounds3 mesh_bounds = ComputeMeshBounds(*mesh_it->second);
    if (!mesh_bounds.valid) {
      continue;
    }
    mesh_bounds.min.x += node.local.translation.x;
    mesh_bounds.min.y += node.local.translation.y;
    mesh_bounds.min.z += node.local.translation.z;
    mesh_bounds.max.x += node.local.translation.x;
    mesh_bounds.max.y += node.local.translation.y;
    mesh_bounds.max.z += node.local.translation.z;
    IncludeBounds(&bounds, mesh_bounds);
  }
  return bounds;
}

std::vector<ObjectId> FindRootNodes(const Scene& scene) {
  std::vector<ObjectId> roots;
  for (const auto& node : scene.nodes) {
    if (node.parent == kInvalidObjectId) {
      roots.push_back(node.id);
    }
  }
  std::sort(roots.begin(), roots.end());
  return roots;
}

std::vector<ObjectId> FindLeafNodes(const Scene& scene) {
  std::vector<ObjectId> leaves;
  for (const auto& node : scene.nodes) {
    if (node.children.empty()) {
      leaves.push_back(node.id);
    }
  }
  std::sort(leaves.begin(), leaves.end());
  return leaves;
}

std::vector<ObjectId> FindReachableNodes(const Scene& scene) {
  auto nodes = NodeIndex(scene);
  std::unordered_set<ObjectId> reached;
  std::deque<ObjectId> queue;
  for (ObjectId root : FindRootNodes(scene)) {
    if (root != kInvalidObjectId) {
      queue.push_back(root);
    }
  }
  while (!queue.empty()) {
    ObjectId id = queue.front();
    queue.pop_front();
    if (!reached.insert(id).second) {
      continue;
    }
    auto node_it = nodes.find(id);
    if (node_it == nodes.end()) {
      continue;
    }
    for (ObjectId child : node_it->second->children) {
      if (child != kInvalidObjectId) {
        queue.push_back(child);
      }
    }
  }
  return SortedIds(std::move(reached));
}

std::vector<ObjectId> FindUnreachableNodes(const Scene& scene) {
  std::unordered_set<ObjectId> reached;
  for (ObjectId id : FindReachableNodes(scene)) {
    reached.insert(id);
  }
  std::vector<ObjectId> unreachable;
  for (const auto& node : scene.nodes) {
    if (!ContainsId(reached, node.id)) {
      unreachable.push_back(node.id);
    }
  }
  std::sort(unreachable.begin(), unreachable.end());
  return unreachable;
}

std::vector<ObjectId> FindReferencedMaterials(const Scene& scene) {
  std::unordered_set<ObjectId> ids;
  for (const auto& mesh : scene.meshes) {
    if (mesh.material != kInvalidObjectId) {
      ids.insert(mesh.material);
    }
  }
  return SortedIds(std::move(ids));
}

std::vector<ObjectId> FindReferencedTextures(const Scene& scene) {
  std::unordered_set<ObjectId> ids;
  for (const auto& material : scene.materials) {
    if (material.base_color_texture != kInvalidObjectId) {
      ids.insert(material.base_color_texture);
    }
  }
  return SortedIds(std::move(ids));
}

std::vector<SceneIssue> AuditSceneStructure(const Scene& scene) {
  std::vector<SceneIssue> issues;
  auto nodes = NodeIndex(scene);
  std::unordered_set<ObjectId> mesh_ids;
  std::unordered_set<ObjectId> material_ids;
  std::unordered_set<ObjectId> texture_ids;
  for (const auto& mesh : scene.meshes) {
    mesh_ids.insert(mesh.id);
  }
  for (const auto& material : scene.materials) {
    material_ids.insert(material.id);
  }
  for (const auto& texture : scene.textures) {
    texture_ids.insert(texture.id);
  }

  for (const auto& node : scene.nodes) {
    if (node.parent != kInvalidObjectId && nodes.find(node.parent) == nodes.end()) {
      issues.push_back({"missing_parent", node.name, "node parent does not exist"});
    }
    for (ObjectId child : node.children) {
      auto child_it = nodes.find(child);
      if (child_it == nodes.end()) {
        issues.push_back({"missing_child", node.name, "node child does not exist"});
      } else if (child_it->second->parent != node.id) {
        issues.push_back({"child_parent_mismatch", node.name,
                          "node child does not point back at the parent"});
      }
    }
    if (node.mesh != kInvalidObjectId && mesh_ids.find(node.mesh) == mesh_ids.end()) {
      issues.push_back({"missing_mesh", node.name, "node mesh does not exist"});
    }
  }

  for (const auto& mesh : scene.meshes) {
    if (mesh.material != kInvalidObjectId && material_ids.find(mesh.material) == material_ids.end()) {
      issues.push_back({"missing_material", mesh.name, "mesh material does not exist"});
    }
    for (std::uint32_t index : mesh.indices) {
      if (index >= mesh.vertices.size()) {
        issues.push_back({"bad_mesh_index", mesh.name, "mesh index exceeds vertex count"});
        break;
      }
    }
  }

  for (const auto& material : scene.materials) {
    if (material.base_color_texture != kInvalidObjectId &&
        texture_ids.find(material.base_color_texture) == texture_ids.end()) {
      issues.push_back({"missing_texture", material.name, "material texture does not exist"});
    }
  }

  for (ObjectId id : FindUnreachableNodes(scene)) {
    issues.push_back({"unreachable_node", std::to_string(id), "node is not reachable from a root"});
  }

  return issues;
}

std::string FormatSceneStats(const SceneStats& stats) {
  std::ostringstream out;
  out << "nodes=" << stats.node_count << "\n";
  out << "root_nodes=" << stats.root_node_count << "\n";
  out << "leaf_nodes=" << stats.leaf_node_count << "\n";
  out << "meshes=" << stats.geometry.mesh_count << "\n";
  out << "vertices=" << stats.geometry.vertex_count << "\n";
  out << "indices=" << stats.geometry.index_count << "\n";
  out << "triangles=" << stats.geometry.triangle_count << "\n";
  out << "materials=" << stats.resources.material_count << "\n";
  out << "textures=" << stats.resources.texture_count << "\n";
  out << "cameras=" << stats.camera_count << "\n";
  out << "lights=" << stats.light_count << "\n";
  out << "skeletons=" << stats.skeleton_count << "\n";
  out << "skeleton_joints=" << stats.skeleton_joint_count << "\n";
  out << "animations=" << stats.animation.animation_count << "\n";
  out << "animation_channels=" << stats.animation.channel_count << "\n";
  out << "animation_keys=" << stats.animation.key_count << "\n";
  out << "longest_animation_seconds=" << FormatFloat(stats.animation.longest_duration_seconds)
      << "\n";
  return out.str();
}

}  // namespace bse
