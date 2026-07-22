#include "bse/scene_ops.hpp"

#include "bse/scene_stats.hpp"
#include "bse/validator.hpp"

#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <utility>

namespace bse {

namespace {

template <typename T>
void SortById(std::vector<T>* values) {
  std::stable_sort(values->begin(), values->end(),
                   [](const T& lhs, const T& rhs) { return lhs.id < rhs.id; });
}

bool ReplaceText(std::string* value, const std::vector<RenameRule>& rules) {
  for (const auto& rule : rules) {
    if (*value == rule.from) {
      *value = rule.to;
      return true;
    }
  }
  return false;
}

std::unordered_set<ObjectId> ReferencedMeshIds(const Scene& scene) {
  std::unordered_set<ObjectId> ids;
  for (const auto& node : scene.nodes) {
    if (node.mesh != kInvalidObjectId) {
      ids.insert(node.mesh);
    }
  }
  return ids;
}

std::unordered_set<ObjectId> ReferencedMaterialIds(const Scene& scene) {
  std::unordered_set<ObjectId> ids;
  for (const auto& mesh : scene.meshes) {
    if (mesh.material != kInvalidObjectId) {
      ids.insert(mesh.material);
    }
  }
  return ids;
}

std::unordered_set<ObjectId> ReferencedTextureIds(const Scene& scene) {
  std::unordered_set<ObjectId> ids;
  for (const auto& material : scene.materials) {
    if (material.base_color_texture != kInvalidObjectId) {
      ids.insert(material.base_color_texture);
    }
  }
  return ids;
}

template <typename T>
std::size_t EraseUnreferenced(std::vector<T>* values, const std::unordered_set<ObjectId>& keep) {
  const auto old_size = values->size();
  values->erase(std::remove_if(values->begin(), values->end(),
                               [&keep](const T& value) {
                                 return keep.find(value.id) == keep.end();
                               }),
                values->end());
  return old_size - values->size();
}

ObjectId MaxObjectId(const Scene& scene) {
  ObjectId max_id = 0;
  auto include = [&max_id](ObjectId id) { max_id = std::max(max_id, id); };
  for (const auto& node : scene.nodes) include(node.id);
  for (const auto& mesh : scene.meshes) include(mesh.id);
  for (const auto& material : scene.materials) include(material.id);
  for (const auto& texture : scene.textures) include(texture.id);
  for (const auto& camera : scene.cameras) include(camera.id);
  for (const auto& light : scene.lights) include(light.id);
  for (const auto& skeleton : scene.skeletons) {
    include(skeleton.id);
    for (const auto& joint : skeleton.joints) include(joint.id);
  }
  for (const auto& animation : scene.animations) include(animation.id);
  return max_id;
}

}  // namespace

Result<Scene> CloneSceneWithName(const Scene& scene, std::string name) {
  Scene clone = scene;
  clone.name = std::move(name);
  auto status = ValidateScene(clone);
  if (!status.ok()) {
    return status;
  }
  return clone;
}

NormalizeReport NormalizeScene(Scene* scene, const NormalizeOptions& options) {
  NormalizeReport report;
  if (options.remove_empty_metadata) {
    for (auto it = scene->metadata.begin(); it != scene->metadata.end();) {
      if (it->first.empty() || it->second.empty()) {
        it = scene->metadata.erase(it);
      } else {
        ++it;
      }
    }
  }
  if (options.rebuild_child_lists) {
    report.rebuilt_child_links = RebuildChildLists(scene);
  }
  if (options.drop_unreferenced_resources) {
    report.removed_meshes = DropUnreferencedMeshes(scene);
    report.removed_materials = DropUnreferencedMaterials(scene);
    report.removed_textures = DropUnreferencedTextures(scene);
  }
  if (options.sort_collections) {
    SortSceneCollections(scene);
  }
  return report;
}

std::size_t ApplyRenameRules(Scene* scene, const std::vector<RenameRule>& rules) {
  std::size_t renamed = 0;
  auto rename = [&renamed, &rules](std::string* value) {
    if (ReplaceText(value, rules)) {
      ++renamed;
    }
  };
  rename(&scene->name);
  for (auto& node : scene->nodes) rename(&node.name);
  for (auto& mesh : scene->meshes) rename(&mesh.name);
  for (auto& material : scene->materials) rename(&material.name);
  for (auto& texture : scene->textures) rename(&texture.name);
  for (auto& camera : scene->cameras) rename(&camera.name);
  for (auto& light : scene->lights) rename(&light.name);
  for (auto& skeleton : scene->skeletons) {
    rename(&skeleton.name);
    for (auto& joint : skeleton.joints) rename(&joint.name);
  }
  for (auto& animation : scene->animations) rename(&animation.name);
  return renamed;
}

std::size_t PrefixResourceUris(Scene* scene, const std::string& prefix) {
  std::size_t changed = 0;
  for (auto& texture : scene->textures) {
    if (!texture.uri.empty() && texture.uri.find("://") == std::string::npos &&
        texture.uri.rfind(prefix, 0) != 0) {
      texture.uri = prefix + texture.uri;
      ++changed;
    }
  }
  return changed;
}

std::size_t RemoveMetadataKeys(Scene* scene, const std::vector<std::string>& keys) {
  std::size_t removed = 0;
  for (const auto& key : keys) {
    removed += scene->metadata.erase(key);
  }
  return removed;
}

std::size_t RebuildChildLists(Scene* scene) {
  for (auto& node : scene->nodes) {
    node.children.clear();
  }
  std::unordered_map<ObjectId, Node*> nodes;
  nodes.reserve(scene->nodes.size());
  for (auto& node : scene->nodes) {
    nodes.emplace(node.id, &node);
  }
  std::size_t linked = 0;
  for (auto& node : scene->nodes) {
    if (node.parent == kInvalidObjectId) {
      continue;
    }
    auto parent = nodes.find(node.parent);
    if (parent != nodes.end()) {
      parent->second->children.push_back(node.id);
      ++linked;
    }
  }
  for (auto& node : scene->nodes) {
    std::sort(node.children.begin(), node.children.end());
  }
  return linked;
}

std::size_t DropUnreferencedMeshes(Scene* scene) {
  return EraseUnreferenced(&scene->meshes, ReferencedMeshIds(*scene));
}

std::size_t DropUnreferencedMaterials(Scene* scene) {
  return EraseUnreferenced(&scene->materials, ReferencedMaterialIds(*scene));
}

std::size_t DropUnreferencedTextures(Scene* scene) {
  return EraseUnreferenced(&scene->textures, ReferencedTextureIds(*scene));
}

void SortSceneCollections(Scene* scene) {
  SortById(&scene->nodes);
  SortById(&scene->meshes);
  SortById(&scene->materials);
  SortById(&scene->textures);
  SortById(&scene->cameras);
  SortById(&scene->lights);
  SortById(&scene->skeletons);
  SortById(&scene->animations);
  for (auto& node : scene->nodes) {
    std::sort(node.children.begin(), node.children.end());
  }
  for (auto& skeleton : scene->skeletons) {
    SortById(&skeleton.joints);
  }
}

Result<Scene> MergeScenes(const Scene& left, const Scene& right, std::string merged_name) {
  Scene merged = left;
  merged.name = std::move(merged_name);

  const ObjectId offset = MaxObjectId(left) + 1U;
  std::unordered_map<ObjectId, ObjectId> node_map;
  for (const auto& node : right.nodes) {
    node_map.emplace(node.id, node.id + offset);
  }

  auto shift = [offset](ObjectId id) {
    return id == kInvalidObjectId ? kInvalidObjectId : id + offset;
  };

  for (auto node : right.nodes) {
    node.id = shift(node.id);
    node.parent = shift(node.parent);
    for (auto& child : node.children) {
      child = shift(child);
    }
    node.mesh = shift(node.mesh);
    node.camera = shift(node.camera);
    node.light = shift(node.light);
    merged.nodes.push_back(std::move(node));
  }
  for (auto mesh : right.meshes) {
    mesh.id = shift(mesh.id);
    mesh.material = shift(mesh.material);
    merged.meshes.push_back(std::move(mesh));
  }
  for (auto material : right.materials) {
    material.id = shift(material.id);
    material.base_color_texture = shift(material.base_color_texture);
    merged.materials.push_back(std::move(material));
  }
  for (auto texture : right.textures) {
    texture.id = shift(texture.id);
    merged.textures.push_back(std::move(texture));
  }
  for (auto camera : right.cameras) {
    camera.id = shift(camera.id);
    merged.cameras.push_back(std::move(camera));
  }
  for (auto light : right.lights) {
    light.id = shift(light.id);
    merged.lights.push_back(std::move(light));
  }
  for (auto skeleton : right.skeletons) {
    skeleton.id = shift(skeleton.id);
    for (auto& joint : skeleton.joints) {
      joint.id = shift(joint.id);
      joint.parent = shift(joint.parent);
    }
    merged.skeletons.push_back(std::move(skeleton));
  }
  for (auto animation : right.animations) {
    animation.id = shift(animation.id);
    for (auto& channel : animation.channels) {
      channel.target_node = shift(channel.target_node);
    }
    merged.animations.push_back(std::move(animation));
  }
  for (const auto& entry : right.metadata) {
    merged.metadata.emplace("right." + entry.first, entry.second);
  }

  NormalizeScene(&merged);
  auto status = ValidateScene(merged);
  if (!status.ok()) {
    return status;
  }
  return merged;
}

}  // namespace bse
