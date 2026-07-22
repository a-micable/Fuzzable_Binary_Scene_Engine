#pragma once

#include "bse/scene.hpp"
#include "bse/scene_diff.hpp"
#include "bse/scene_stats.hpp"
#include "bse/status.hpp"

#include <string>
#include <unordered_map>
#include <vector>

namespace bse {

struct SceneCollectionEntry {
  std::string key;
  Scene scene;
};

struct CollectionStats {
  std::size_t scene_count = 0;
  std::size_t total_nodes = 0;
  std::size_t total_meshes = 0;
  std::size_t total_vertices = 0;
  std::size_t total_indices = 0;
  std::size_t total_animations = 0;
};

class SceneCollection {
 public:
  bool Add(std::string key, Scene scene);
  bool Remove(const std::string& key);
  Scene* Find(const std::string& key);
  const Scene* Find(const std::string& key) const;
  std::vector<std::string> Keys() const;
  std::vector<SceneCollectionEntry> Entries() const;
  CollectionStats Stats() const;
  std::vector<std::string> FindScenesUsingTexture(const std::string& uri_fragment) const;
  std::vector<std::string> FindScenesWithAnimation() const;
  Result<SceneDiff> Diff(const std::string& left, const std::string& right) const;
  Result<Scene> MergeAll(std::string name) const;
  std::string Summary() const;

 private:
  std::unordered_map<std::string, Scene> scenes_;
};

}  // namespace bse
