#include "bse/scene_collection.hpp"

#include "bse/scene_ops.hpp"
#include "bse/validator.hpp"

#include <algorithm>
#include <sstream>

namespace bse {

bool SceneCollection::Add(std::string key, Scene scene) {
  if (key.empty()) {
    return false;
  }
  auto status = ValidateScene(scene);
  if (!status.ok()) {
    return false;
  }
  scenes_[std::move(key)] = std::move(scene);
  return true;
}

bool SceneCollection::Remove(const std::string& key) {
  return scenes_.erase(key) != 0;
}

Scene* SceneCollection::Find(const std::string& key) {
  auto it = scenes_.find(key);
  return it == scenes_.end() ? nullptr : &it->second;
}

const Scene* SceneCollection::Find(const std::string& key) const {
  auto it = scenes_.find(key);
  return it == scenes_.end() ? nullptr : &it->second;
}

std::vector<std::string> SceneCollection::Keys() const {
  std::vector<std::string> keys;
  keys.reserve(scenes_.size());
  for (const auto& entry : scenes_) {
    keys.push_back(entry.first);
  }
  std::sort(keys.begin(), keys.end());
  return keys;
}

std::vector<SceneCollectionEntry> SceneCollection::Entries() const {
  std::vector<SceneCollectionEntry> entries;
  for (const auto& key : Keys()) {
    entries.push_back({key, scenes_.at(key)});
  }
  return entries;
}

CollectionStats SceneCollection::Stats() const {
  CollectionStats stats;
  stats.scene_count = scenes_.size();
  for (const auto& entry : scenes_) {
    const auto scene_stats = ComputeSceneStats(entry.second);
    stats.total_nodes += scene_stats.node_count;
    stats.total_meshes += scene_stats.geometry.mesh_count;
    stats.total_vertices += scene_stats.geometry.vertex_count;
    stats.total_indices += scene_stats.geometry.index_count;
    stats.total_animations += scene_stats.animation.animation_count;
  }
  return stats;
}

std::vector<std::string> SceneCollection::FindScenesUsingTexture(
    const std::string& uri_fragment) const {
  std::vector<std::string> matches;
  for (const auto& entry : scenes_) {
    for (const auto& texture : entry.second.textures) {
      if (texture.uri.find(uri_fragment) != std::string::npos) {
        matches.push_back(entry.first);
        break;
      }
    }
  }
  std::sort(matches.begin(), matches.end());
  return matches;
}

std::vector<std::string> SceneCollection::FindScenesWithAnimation() const {
  std::vector<std::string> matches;
  for (const auto& entry : scenes_) {
    if (!entry.second.animations.empty()) {
      matches.push_back(entry.first);
    }
  }
  std::sort(matches.begin(), matches.end());
  return matches;
}

Result<SceneDiff> SceneCollection::Diff(const std::string& left, const std::string& right) const {
  const Scene* left_scene = Find(left);
  const Scene* right_scene = Find(right);
  if (left_scene == nullptr || right_scene == nullptr) {
    return Status::Error(ErrorCode::kMissingReference, "scene key not found in collection");
  }
  return DiffScenes(*left_scene, *right_scene);
}

Result<Scene> SceneCollection::MergeAll(std::string name) const {
  if (scenes_.empty()) {
    return Status::Error(ErrorCode::kValidationFailed, "cannot merge an empty scene collection");
  }
  auto keys = Keys();
  Scene merged = scenes_.at(keys.front());
  merged.name = std::move(name);
  for (std::size_t i = 1; i < keys.size(); ++i) {
    auto next = MergeScenes(merged, scenes_.at(keys[i]), merged.name);
    if (!next.ok()) {
      return next.status();
    }
    merged = std::move(next.value());
  }
  return merged;
}

std::string SceneCollection::Summary() const {
  const auto stats = Stats();
  std::ostringstream out;
  out << "scenes=" << stats.scene_count << "\n";
  out << "nodes=" << stats.total_nodes << "\n";
  out << "meshes=" << stats.total_meshes << "\n";
  out << "vertices=" << stats.total_vertices << "\n";
  out << "indices=" << stats.total_indices << "\n";
  out << "animations=" << stats.total_animations << "\n";
  return out.str();
}

}  // namespace bse
