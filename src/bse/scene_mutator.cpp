#include "bse/scene_mutator.hpp"

#include "bse/scene_stats.hpp"
#include "bse/validator.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <sstream>
#include <unordered_map>
#include <unordered_set>

namespace bse {

namespace {

class DeterministicRng {
 public:
  explicit DeterministicRng(std::uint64_t seed) : state_(seed == 0 ? 0xA5A5A5A5A5A5A5A5ULL : seed) {}

  std::uint32_t NextU32() {
    state_ ^= state_ << 13U;
    state_ ^= state_ >> 7U;
    state_ ^= state_ << 17U;
    return static_cast<std::uint32_t>((state_ >> 16U) ^ state_);
  }

  std::size_t Index(std::size_t size) {
    if (size == 0) return 0;
    return static_cast<std::size_t>(NextU32()) % size;
  }

  float Unit() {
    constexpr float kScale = 1.0F / static_cast<float>(std::numeric_limits<std::uint32_t>::max());
    return static_cast<float>(NextU32()) * kScale;
  }

  float Signed(float magnitude) {
    return (Unit() * 2.0F - 1.0F) * magnitude;
  }

 private:
  std::uint64_t state_ = 0;
};

ObjectId MaxId(const Scene& scene) {
  ObjectId max_id = 0;
  auto visit = [&max_id](ObjectId id) {
    if (id > max_id) max_id = id;
  };
  for (const auto& node : scene.nodes) visit(node.id);
  for (const auto& mesh : scene.meshes) visit(mesh.id);
  for (const auto& material : scene.materials) visit(material.id);
  for (const auto& texture : scene.textures) visit(texture.id);
  for (const auto& camera : scene.cameras) visit(camera.id);
  for (const auto& light : scene.lights) visit(light.id);
  for (const auto& skeleton : scene.skeletons) {
    visit(skeleton.id);
    for (const auto& joint : skeleton.joints) visit(joint.id);
  }
  for (const auto& animation : scene.animations) visit(animation.id);
  return max_id;
}

ObjectId AllocateId(ObjectId* next_id) {
  ++(*next_id);
  if (*next_id == kInvalidObjectId) ++(*next_id);
  return *next_id;
}

const Node* FindNode(const Scene& scene, ObjectId id) {
  for (const auto& node : scene.nodes) {
    if (node.id == id) return &node;
  }
  return nullptr;
}

void RebuildChildren(Scene* scene) {
  for (auto& node : scene->nodes) node.children.clear();
  std::unordered_map<ObjectId, Node*> by_id;
  for (auto& node : scene->nodes) by_id[node.id] = &node;
  for (const auto& node : scene->nodes) {
    if (node.parent == kInvalidObjectId) continue;
    auto parent = by_id.find(node.parent);
    if (parent != by_id.end()) parent->second->children.push_back(node.id);
  }
  for (auto& node : scene->nodes) {
    std::sort(node.children.begin(), node.children.end());
    node.children.erase(std::unique(node.children.begin(), node.children.end()), node.children.end());
  }
}

std::string DecoratedName(const std::string& name, const std::string& suffix, std::size_t index) {
  std::ostringstream out;
  out << (name.empty() ? "unnamed" : name) << "_" << suffix << "_" << index;
  return out.str();
}

float Quantize(float value, float step) {
  if (step <= 0.0F) return value;
  return std::round(value / step) * step;
}

void QuantizeVec3(Vec3* vec, float step) {
  vec->x = Quantize(vec->x, step);
  vec->y = Quantize(vec->y, step);
  vec->z = Quantize(vec->z, step);
}

void AddStep(MutationReport* report, MutationKind kind, std::string subject, std::string note) {
  report->steps.push_back({kind, std::move(subject), std::move(note)});
}

void RenameObjects(Scene* scene, MutationReport* report) {
  std::size_t index = 0;
  for (auto& node : scene->nodes) node.name = DecoratedName(node.name, "mut_node", index++);
  index = 0;
  for (auto& mesh : scene->meshes) mesh.name = DecoratedName(mesh.name, "mut_mesh", index++);
  index = 0;
  for (auto& material : scene->materials) material.name = DecoratedName(material.name, "mut_mat", index++);
  index = 0;
  for (auto& texture : scene->textures) texture.name = DecoratedName(texture.name, "mut_tex", index++);
  AddStep(report, MutationKind::kRenameObjects, scene->name, "renamed scene objects deterministically");
}

void PerturbTransforms(Scene* scene, DeterministicRng* rng, const MutationOptions& options,
                       MutationReport* report) {
  for (auto& node : scene->nodes) {
    node.local.translation.x += rng->Signed(options.transform_jitter);
    node.local.translation.y += rng->Signed(options.transform_jitter);
    node.local.translation.z += rng->Signed(options.transform_jitter);
    node.local.scale.x = std::max(0.001F, node.local.scale.x + rng->Signed(options.transform_jitter * 0.1F));
    node.local.scale.y = std::max(0.001F, node.local.scale.y + rng->Signed(options.transform_jitter * 0.1F));
    node.local.scale.z = std::max(0.001F, node.local.scale.z + rng->Signed(options.transform_jitter * 0.1F));
  }
  AddStep(report, MutationKind::kPerturbTransforms, scene->name, "jittered local transforms");
}

void PerturbGeometry(Scene* scene, DeterministicRng* rng, const MutationOptions& options,
                     MutationReport* report) {
  std::size_t changed = 0;
  for (auto& mesh : scene->meshes) {
    for (auto& vertex : mesh.vertices) {
      vertex.position.x += rng->Signed(options.vertex_jitter);
      vertex.position.y += rng->Signed(options.vertex_jitter);
      vertex.position.z += rng->Signed(options.vertex_jitter);
      ++changed;
    }
  }
  report->vertices_changed += changed;
  AddStep(report, MutationKind::kPerturbGeometry, scene->name, "jittered vertex positions");
}

void QuantizeGeometry(Scene* scene, const MutationOptions& options, MutationReport* report) {
  std::size_t changed = 0;
  for (auto& mesh : scene->meshes) {
    for (auto& vertex : mesh.vertices) {
      QuantizeVec3(&vertex.position, options.quantize_step);
      QuantizeVec3(&vertex.normal, options.quantize_step);
      vertex.texcoord.x = Quantize(vertex.texcoord.x, options.quantize_step);
      vertex.texcoord.y = Quantize(vertex.texcoord.y, options.quantize_step);
      ++changed;
    }
  }
  report->vertices_changed += changed;
  AddStep(report, MutationKind::kQuantizeGeometry, scene->name, "quantized mesh attributes");
}

void DuplicateMesh(Scene* scene, ObjectId* next_id, DeterministicRng* rng,
                   const MutationOptions& options, MutationReport* report) {
  if (scene->meshes.empty() || report->meshes_added >= options.max_new_meshes) return;
  const Mesh source = scene->meshes[rng->Index(scene->meshes.size())];
  Mesh copy = source;
  copy.id = AllocateId(next_id);
  copy.name = DecoratedName(source.name, "copy", report->meshes_added);
  scene->meshes.push_back(copy);
  ++report->meshes_added;
  for (auto& node : scene->nodes) {
    if (node.mesh == source.id && rng->Index(2) == 0) node.mesh = copy.id;
  }
  AddStep(report, MutationKind::kDuplicateMesh, source.name, "duplicated mesh and retargeted some nodes");
}

void DuplicateSubtree(Scene* scene, ObjectId* next_id, DeterministicRng* rng,
                      const MutationOptions& options, MutationReport* report) {
  if (scene->nodes.empty() || report->nodes_added >= options.max_new_nodes) return;
  RebuildChildren(scene);
  const Node source = scene->nodes[rng->Index(scene->nodes.size())];
  Node copy = source;
  copy.id = AllocateId(next_id);
  copy.parent = source.parent;
  copy.children.clear();
  copy.name = DecoratedName(source.name, "branch", report->nodes_added);
  copy.local.translation.x += 1.0F + static_cast<float>(report->nodes_added);
  scene->nodes.push_back(copy);
  ++report->nodes_added;

  const std::size_t remaining = options.max_new_nodes - report->nodes_added;
  const std::size_t child_budget = std::min<std::size_t>(source.children.size(), remaining);
  for (std::size_t i = 0; i < child_budget; ++i) {
    const Node* child = FindNode(*scene, source.children[i]);
    if (child == nullptr) continue;
    Node child_copy = *child;
    child_copy.id = AllocateId(next_id);
    child_copy.parent = copy.id;
    child_copy.children.clear();
    child_copy.name = DecoratedName(child->name, "branch_child", i);
    scene->nodes.push_back(child_copy);
    ++report->nodes_added;
  }
  RebuildChildren(scene);
  AddStep(report, MutationKind::kDuplicateSubtree, source.name, "duplicated a shallow hierarchy branch");
}

void DropLeafNodes(Scene* scene, DeterministicRng* rng, MutationReport* report) {
  RebuildChildren(scene);
  if (scene->nodes.size() <= 1) return;
  const auto roots = FindRootNodes(*scene);
  std::unordered_set<ObjectId> protected_ids(roots.begin(), roots.end());
  std::vector<ObjectId> candidates;
  for (const auto& node : scene->nodes) {
    if (node.children.empty() && protected_ids.find(node.id) == protected_ids.end()) {
      candidates.push_back(node.id);
    }
  }
  if (candidates.empty()) return;
  const ObjectId remove_id = candidates[rng->Index(candidates.size())];
  const auto before = scene->nodes.size();
  scene->nodes.erase(std::remove_if(scene->nodes.begin(), scene->nodes.end(),
                                    [remove_id](const Node& node) { return node.id == remove_id; }),
                     scene->nodes.end());
  report->nodes_removed += before - scene->nodes.size();
  RebuildChildren(scene);
  AddStep(report, MutationKind::kDropLeafNodes, std::to_string(remove_id), "dropped a leaf node");
}

void ReassignMaterials(Scene* scene, DeterministicRng* rng, MutationReport* report) {
  if (scene->materials.empty()) return;
  for (auto& mesh : scene->meshes) {
    mesh.material = scene->materials[rng->Index(scene->materials.size())].id;
  }
  AddStep(report, MutationKind::kReassignMaterials, scene->name, "reassigned mesh materials");
}

void ExpandAnimationKeys(Scene* scene, DeterministicRng* rng, const MutationOptions& options,
                         MutationReport* report) {
  for (auto& animation : scene->animations) {
    for (auto& channel : animation.channels) {
      if (channel.keys.empty()) continue;
      const std::size_t add_count = std::min<std::size_t>(options.max_new_keys_per_channel, channel.keys.size());
      for (std::size_t i = 0; i < add_count; ++i) {
        AnimationKey key = channel.keys[rng->Index(channel.keys.size())];
        key.time_seconds = std::min(animation.duration_seconds, key.time_seconds + 0.001F * static_cast<float>(i + 1));
        key.transform.translation.x += rng->Signed(options.transform_jitter);
        channel.keys.push_back(key);
        ++report->animation_keys_added;
      }
      std::sort(channel.keys.begin(), channel.keys.end(),
                [](const AnimationKey& left, const AnimationKey& right) {
                  return left.time_seconds < right.time_seconds;
                });
    }
  }
  AddStep(report, MutationKind::kExpandAnimationKeys, scene->name, "inserted nearby animation keys");
}

void InjectDanglingReferences(Scene* scene, ObjectId* next_id, MutationReport* report) {
  const ObjectId missing = AllocateId(next_id);
  if (!scene->nodes.empty()) scene->nodes.front().mesh = missing;
  if (!scene->meshes.empty()) scene->meshes.front().material = missing + 1U;
  if (!scene->materials.empty()) scene->materials.front().base_color_texture = missing + 2U;
  AddStep(report, MutationKind::kInjectDanglingReferences, std::to_string(missing),
          "introduced missing object references for negative validation cases");
}

Status ApplyMutation(Scene* scene, MutationKind kind, ObjectId* next_id, DeterministicRng* rng,
                     const MutationOptions& options, MutationReport* report) {
  switch (kind) {
    case MutationKind::kRenameObjects:
      RenameObjects(scene, report);
      break;
    case MutationKind::kPerturbTransforms:
      PerturbTransforms(scene, rng, options, report);
      break;
    case MutationKind::kPerturbGeometry:
      PerturbGeometry(scene, rng, options, report);
      break;
    case MutationKind::kDuplicateMesh:
      DuplicateMesh(scene, next_id, rng, options, report);
      break;
    case MutationKind::kDuplicateSubtree:
      DuplicateSubtree(scene, next_id, rng, options, report);
      break;
    case MutationKind::kDropLeafNodes:
      DropLeafNodes(scene, rng, report);
      break;
    case MutationKind::kReassignMaterials:
      ReassignMaterials(scene, rng, report);
      break;
    case MutationKind::kExpandAnimationKeys:
      ExpandAnimationKeys(scene, rng, options, report);
      break;
    case MutationKind::kInjectDanglingReferences:
      if (options.preserve_validity) {
        AddStep(report, kind, scene->name, "skipped dangling references because validity is preserved");
      } else {
        InjectDanglingReferences(scene, next_id, report);
      }
      break;
    case MutationKind::kQuantizeGeometry:
      QuantizeGeometry(scene, options, report);
      break;
  }
  return Status::Ok();
}

}  // namespace

std::string MutationKindName(MutationKind kind) {
  switch (kind) {
    case MutationKind::kRenameObjects:
      return "rename-objects";
    case MutationKind::kPerturbTransforms:
      return "perturb-transforms";
    case MutationKind::kPerturbGeometry:
      return "perturb-geometry";
    case MutationKind::kDuplicateMesh:
      return "duplicate-mesh";
    case MutationKind::kDuplicateSubtree:
      return "duplicate-subtree";
    case MutationKind::kDropLeafNodes:
      return "drop-leaf-nodes";
    case MutationKind::kReassignMaterials:
      return "reassign-materials";
    case MutationKind::kExpandAnimationKeys:
      return "expand-animation-keys";
    case MutationKind::kInjectDanglingReferences:
      return "inject-dangling-references";
    case MutationKind::kQuantizeGeometry:
      return "quantize-geometry";
  }
  return "unknown";
}

std::vector<MutationRecipe> DefaultMutationRecipes() {
  return {
      {"renamed-jittered", {MutationKind::kRenameObjects, MutationKind::kPerturbTransforms,
                            MutationKind::kPerturbGeometry}},
      {"geometry-copies", {MutationKind::kDuplicateMesh, MutationKind::kReassignMaterials,
                           MutationKind::kQuantizeGeometry}},
      {"hierarchy-growth", {MutationKind::kDuplicateSubtree, MutationKind::kDuplicateSubtree,
                            MutationKind::kPerturbTransforms}},
      {"animation-dense", {MutationKind::kExpandAnimationKeys, MutationKind::kPerturbTransforms}},
      {"negative-references", {MutationKind::kInjectDanglingReferences}},
  };
}

Result<MutatedScene> MutateScene(const Scene& source, const MutationRecipe& recipe,
                                 const MutationOptions& options) {
  MutatedScene mutated;
  mutated.name = recipe.name;
  mutated.scene = source;
  mutated.scene.name = source.name + "-" + recipe.name;
  mutated.report.seed = options.seed;
  DeterministicRng rng(options.seed ^ static_cast<std::uint64_t>(recipe.name.size() * 131U));
  ObjectId next_id = MaxId(mutated.scene);

  for (MutationKind kind : recipe.steps) {
    Status status = ApplyMutation(&mutated.scene, kind, &next_id, &rng, options, &mutated.report);
    if (!status.ok()) return status;
  }
  RebuildChildren(&mutated.scene);
  if (options.preserve_validity) {
    Status valid = ValidateScene(mutated.scene);
    if (!valid.ok()) return valid;
  }
  return mutated;
}

std::vector<MutatedScene> GenerateMutationSuite(const Scene& source, const MutationOptions& options) {
  std::vector<MutatedScene> scenes;
  std::uint64_t seed_offset = 0;
  for (const auto& recipe : DefaultMutationRecipes()) {
    MutationOptions local = options;
    local.seed += seed_offset++;
    if (recipe.name == "negative-references") local.preserve_validity = false;
    auto mutated = MutateScene(source, recipe, local);
    if (mutated.ok()) scenes.push_back(mutated.value());
  }
  return scenes;
}

std::string FormatMutationReport(const MutationReport& report) {
  std::ostringstream out;
  out << "seed=" << report.seed << "\n";
  out << "nodes_added=" << report.nodes_added << "\n";
  out << "nodes_removed=" << report.nodes_removed << "\n";
  out << "meshes_added=" << report.meshes_added << "\n";
  out << "vertices_changed=" << report.vertices_changed << "\n";
  out << "animation_keys_added=" << report.animation_keys_added << "\n";
  for (const auto& step : report.steps) {
    out << "- " << MutationKindName(step.kind) << " " << step.subject;
    if (!step.note.empty()) out << ": " << step.note;
    out << "\n";
  }
  return out.str();
}

}  // namespace bse
