#include "bse/scene_optimizer.hpp"

#include "bse/scene_ops.hpp"
#include "bse/validator.hpp"

#include <algorithm>
#include <cmath>
#include <sstream>
#include <unordered_map>
#include <unordered_set>

namespace bse {

namespace {

void AddAction(OptimizationReport* report, OptimizationPass pass, std::string subject,
               std::string detail, std::size_t count) {
  if (count == 0) return;
  report->actions.push_back({pass, std::move(subject), std::move(detail), count});
}

float DistanceSquared(Vec3 left, Vec3 right) {
  const float dx = left.x - right.x;
  const float dy = left.y - right.y;
  const float dz = left.z - right.z;
  return dx * dx + dy * dy + dz * dz;
}

bool SamePoint(Vec3 left, Vec3 right, float epsilon) {
  return DistanceSquared(left, right) <= epsilon * epsilon;
}

bool DegenerateTriangle(const Mesh& mesh, std::uint32_t a, std::uint32_t b, std::uint32_t c,
                        float epsilon) {
  if (a >= mesh.vertices.size() || b >= mesh.vertices.size() || c >= mesh.vertices.size()) return true;
  const Vec3 pa = mesh.vertices[a].position;
  const Vec3 pb = mesh.vertices[b].position;
  const Vec3 pc = mesh.vertices[c].position;
  return SamePoint(pa, pb, epsilon) || SamePoint(pb, pc, epsilon) || SamePoint(pa, pc, epsilon);
}

std::size_t RemoveDegenerateTriangles(Scene* scene, float epsilon) {
  std::size_t removed = 0;
  for (auto& mesh : scene->meshes) {
    std::vector<std::uint32_t> filtered;
    filtered.reserve(mesh.indices.size());
    for (std::size_t i = 0; i + 2U < mesh.indices.size(); i += 3U) {
      const std::uint32_t a = mesh.indices[i];
      const std::uint32_t b = mesh.indices[i + 1U];
      const std::uint32_t c = mesh.indices[i + 2U];
      if (DegenerateTriangle(mesh, a, b, c, epsilon)) {
        ++removed;
        continue;
      }
      filtered.push_back(a);
      filtered.push_back(b);
      filtered.push_back(c);
    }
    const std::size_t trailing = mesh.indices.size() % 3U;
    if (trailing != 0U) {
      removed += 1U;
    }
    mesh.indices = std::move(filtered);
  }
  return removed;
}

std::string TextureKey(const Texture& texture) {
  std::ostringstream out;
  out << texture.uri << "|" << texture.width << "|" << texture.height << "|" << texture.color_space;
  return out.str();
}

std::string VecKey(Vec3 value) {
  std::ostringstream out;
  out << value.x << "," << value.y << "," << value.z;
  return out.str();
}

std::string MaterialKey(const Material& material) {
  std::ostringstream out;
  out << VecKey(material.base_color) << "|" << material.roughness << "|" << material.metallic
      << "|" << material.base_color_texture;
  return out.str();
}

std::size_t MergeDuplicateTextures(Scene* scene) {
  std::unordered_map<std::string, ObjectId> canonical;
  std::unordered_map<ObjectId, ObjectId> remap;
  std::vector<Texture> unique;
  unique.reserve(scene->textures.size());
  for (const auto& texture : scene->textures) {
    const std::string key = TextureKey(texture);
    auto found = canonical.find(key);
    if (found == canonical.end()) {
      canonical.emplace(key, texture.id);
      unique.push_back(texture);
    } else {
      remap.emplace(texture.id, found->second);
    }
  }
  if (remap.empty()) return 0;
  for (auto& material : scene->materials) {
    auto found = remap.find(material.base_color_texture);
    if (found != remap.end()) material.base_color_texture = found->second;
  }
  const std::size_t removed = scene->textures.size() - unique.size();
  scene->textures = std::move(unique);
  return removed;
}

std::size_t MergeDuplicateMaterials(Scene* scene) {
  std::unordered_map<std::string, ObjectId> canonical;
  std::unordered_map<ObjectId, ObjectId> remap;
  std::vector<Material> unique;
  unique.reserve(scene->materials.size());
  for (const auto& material : scene->materials) {
    const std::string key = MaterialKey(material);
    auto found = canonical.find(key);
    if (found == canonical.end()) {
      canonical.emplace(key, material.id);
      unique.push_back(material);
    } else {
      remap.emplace(material.id, found->second);
    }
  }
  if (remap.empty()) return 0;
  for (auto& mesh : scene->meshes) {
    auto found = remap.find(mesh.material);
    if (found != remap.end()) mesh.material = found->second;
  }
  const std::size_t removed = scene->materials.size() - unique.size();
  scene->materials = std::move(unique);
  return removed;
}

std::size_t StripEmptyAnimations(Scene* scene) {
  const auto before = scene->animations.size();
  scene->animations.erase(std::remove_if(scene->animations.begin(), scene->animations.end(),
                                         [](const Animation& animation) {
                                           if (animation.channels.empty()) return true;
                                           return std::all_of(animation.channels.begin(), animation.channels.end(),
                                                              [](const AnimationChannel& channel) {
                                                                return channel.keys.empty();
                                                              });
                                         }),
                          scene->animations.end());
  return before - scene->animations.size();
}

std::size_t StripEmptySkeletons(Scene* scene) {
  const auto before = scene->skeletons.size();
  scene->skeletons.erase(std::remove_if(scene->skeletons.begin(), scene->skeletons.end(),
                                        [](const Skeleton& skeleton) {
                                          return skeleton.joints.empty();
                                        }),
                         scene->skeletons.end());
  return before - scene->skeletons.size();
}

void AddId(std::vector<ObjectId>* ids, ObjectId id) {
  if (id != kInvalidObjectId) ids->push_back(id);
}

std::unordered_map<ObjectId, ObjectId> BuildCompactionMap(const Scene& scene) {
  std::vector<ObjectId> ids;
  ids.reserve(scene.nodes.size() + scene.meshes.size() + scene.materials.size() + scene.textures.size() +
              scene.cameras.size() + scene.lights.size() + scene.skeletons.size() + scene.animations.size());
  for (const auto& node : scene.nodes) AddId(&ids, node.id);
  for (const auto& mesh : scene.meshes) AddId(&ids, mesh.id);
  for (const auto& material : scene.materials) AddId(&ids, material.id);
  for (const auto& texture : scene.textures) AddId(&ids, texture.id);
  for (const auto& camera : scene.cameras) AddId(&ids, camera.id);
  for (const auto& light : scene.lights) AddId(&ids, light.id);
  for (const auto& skeleton : scene.skeletons) {
    AddId(&ids, skeleton.id);
    for (const auto& joint : skeleton.joints) AddId(&ids, joint.id);
  }
  for (const auto& animation : scene.animations) AddId(&ids, animation.id);
  std::sort(ids.begin(), ids.end());
  ids.erase(std::unique(ids.begin(), ids.end()), ids.end());
  std::unordered_map<ObjectId, ObjectId> remap;
  ObjectId next = 1;
  for (ObjectId id : ids) remap.emplace(id, next++);
  return remap;
}

ObjectId RemapId(ObjectId id, const std::unordered_map<ObjectId, ObjectId>& remap) {
  if (id == kInvalidObjectId) return id;
  auto found = remap.find(id);
  return found == remap.end() ? id : found->second;
}

std::size_t CompactObjectIds(Scene* scene) {
  const auto remap = BuildCompactionMap(*scene);
  std::size_t changed = 0;
  auto apply = [&remap, &changed](ObjectId* id) {
    const ObjectId old = *id;
    *id = RemapId(*id, remap);
    if (*id != old) ++changed;
  };
  for (auto& node : scene->nodes) {
    apply(&node.id);
    apply(&node.parent);
    apply(&node.mesh);
    apply(&node.camera);
    apply(&node.light);
    for (auto& child : node.children) apply(&child);
  }
  for (auto& mesh : scene->meshes) {
    apply(&mesh.id);
    apply(&mesh.material);
  }
  for (auto& material : scene->materials) {
    apply(&material.id);
    apply(&material.base_color_texture);
  }
  for (auto& texture : scene->textures) apply(&texture.id);
  for (auto& camera : scene->cameras) apply(&camera.id);
  for (auto& light : scene->lights) apply(&light.id);
  for (auto& skeleton : scene->skeletons) {
    apply(&skeleton.id);
    for (auto& joint : skeleton.joints) {
      apply(&joint.id);
      apply(&joint.parent);
    }
  }
  for (auto& animation : scene->animations) {
    apply(&animation.id);
    for (auto& channel : animation.channels) apply(&channel.target_node);
  }
  RebuildChildLists(scene);
  return changed;
}

Status ValidateIfRequested(const Scene& scene, const OptimizationOptions& options) {
  if (!options.validate_after_pass) return Status::Ok();
  return ValidateScene(scene);
}

Status RunPass(Scene* scene, OptimizationPass pass, const OptimizationOptions& options,
               OptimizationReport* report) {
  switch (pass) {
    case OptimizationPass::kRemoveDegenerateTriangles: {
      const std::size_t removed = RemoveDegenerateTriangles(scene, options.position_epsilon);
      report->removed_triangles += removed;
      AddAction(report, pass, "meshes", "removed degenerate or incomplete triangles", removed);
      break;
    }
    case OptimizationPass::kRemoveUnusedResources: {
      const std::size_t meshes = DropUnreferencedMeshes(scene);
      const std::size_t materials = DropUnreferencedMaterials(scene);
      const std::size_t textures = DropUnreferencedTextures(scene);
      report->removed_meshes += meshes;
      report->removed_materials += materials;
      report->removed_textures += textures;
      AddAction(report, pass, "meshes", "removed unreferenced meshes", meshes);
      AddAction(report, pass, "materials", "removed unreferenced materials", materials);
      AddAction(report, pass, "textures", "removed unreferenced textures", textures);
      break;
    }
    case OptimizationPass::kMergeDuplicateTextures: {
      const std::size_t removed = MergeDuplicateTextures(scene);
      report->merged_textures += removed;
      AddAction(report, pass, "textures", "merged duplicate texture descriptors", removed);
      break;
    }
    case OptimizationPass::kMergeDuplicateMaterials: {
      const std::size_t removed = MergeDuplicateMaterials(scene);
      report->merged_materials += removed;
      AddAction(report, pass, "materials", "merged duplicate material descriptors", removed);
      break;
    }
    case OptimizationPass::kCompactObjectIds: {
      const std::size_t changed = CompactObjectIds(scene);
      report->remapped_ids += changed;
      AddAction(report, pass, "scene", "compacted object identifier space", changed);
      break;
    }
    case OptimizationPass::kStripEmptyAnimations: {
      const std::size_t removed = StripEmptyAnimations(scene);
      report->removed_animations += removed;
      AddAction(report, pass, "animations", "removed empty animation containers", removed);
      break;
    }
    case OptimizationPass::kStripEmptySkeletons: {
      const std::size_t removed = StripEmptySkeletons(scene);
      report->removed_skeletons += removed;
      AddAction(report, pass, "skeletons", "removed skeletons without joints", removed);
      break;
    }
    case OptimizationPass::kSortCollections:
      SortSceneCollections(scene);
      AddAction(report, pass, "scene", "sorted object collections", 1);
      break;
  }
  return ValidateIfRequested(*scene, options);
}

}  // namespace

std::vector<OptimizationPass> DefaultOptimizationPasses() {
  return {OptimizationPass::kRemoveDegenerateTriangles, OptimizationPass::kMergeDuplicateTextures,
          OptimizationPass::kMergeDuplicateMaterials, OptimizationPass::kStripEmptyAnimations,
          OptimizationPass::kStripEmptySkeletons, OptimizationPass::kRemoveUnusedResources,
          OptimizationPass::kCompactObjectIds, OptimizationPass::kSortCollections};
}

std::string OptimizationPassName(OptimizationPass pass) {
  switch (pass) {
    case OptimizationPass::kRemoveDegenerateTriangles:
      return "remove-degenerate-triangles";
    case OptimizationPass::kRemoveUnusedResources:
      return "remove-unused-resources";
    case OptimizationPass::kMergeDuplicateTextures:
      return "merge-duplicate-textures";
    case OptimizationPass::kMergeDuplicateMaterials:
      return "merge-duplicate-materials";
    case OptimizationPass::kCompactObjectIds:
      return "compact-object-ids";
    case OptimizationPass::kStripEmptyAnimations:
      return "strip-empty-animations";
    case OptimizationPass::kStripEmptySkeletons:
      return "strip-empty-skeletons";
    case OptimizationPass::kSortCollections:
      return "sort-collections";
  }
  return "unknown";
}

Result<OptimizedScene> OptimizeScene(const Scene& scene, const OptimizationOptions& options) {
  OptimizedScene optimized;
  optimized.scene = scene;
  OptimizationOptions local = options;
  if (local.passes.empty()) local.passes = DefaultOptimizationPasses();
  for (OptimizationPass pass : local.passes) {
    Status status = RunPass(&optimized.scene, pass, local, &optimized.report);
    if (!status.ok()) return status;
  }
  return optimized;
}

std::string FormatOptimizationReport(const OptimizationReport& report) {
  std::ostringstream out;
  out << "removed_triangles=" << report.removed_triangles << "\n";
  out << "removed_meshes=" << report.removed_meshes << "\n";
  out << "removed_materials=" << report.removed_materials << "\n";
  out << "removed_textures=" << report.removed_textures << "\n";
  out << "removed_animations=" << report.removed_animations << "\n";
  out << "removed_skeletons=" << report.removed_skeletons << "\n";
  out << "merged_materials=" << report.merged_materials << "\n";
  out << "merged_textures=" << report.merged_textures << "\n";
  out << "remapped_ids=" << report.remapped_ids << "\n";
  for (const auto& action : report.actions) {
    out << "- " << OptimizationPassName(action.pass) << " " << action.subject << " "
        << action.count << ": " << action.detail << "\n";
  }
  return out.str();
}

}  // namespace bse
