#pragma once

#include "bse/scene.hpp"
#include "bse/status.hpp"

#include <cstddef>
#include <string>
#include <vector>

namespace bse {

struct RenameRule {
  std::string from;
  std::string to;
};

struct NormalizeOptions {
  bool sort_collections = true;
  bool rebuild_child_lists = true;
  bool drop_unreferenced_resources = false;
  bool remove_empty_metadata = true;
};

struct NormalizeReport {
  std::size_t renamed_objects = 0;
  std::size_t rebuilt_child_links = 0;
  std::size_t removed_materials = 0;
  std::size_t removed_textures = 0;
  std::size_t removed_meshes = 0;
};

Result<Scene> CloneSceneWithName(const Scene& scene, std::string name);
NormalizeReport NormalizeScene(Scene* scene, const NormalizeOptions& options = NormalizeOptions{});
std::size_t ApplyRenameRules(Scene* scene, const std::vector<RenameRule>& rules);
std::size_t PrefixResourceUris(Scene* scene, const std::string& prefix);
std::size_t RemoveMetadataKeys(Scene* scene, const std::vector<std::string>& keys);
std::size_t RebuildChildLists(Scene* scene);
std::size_t DropUnreferencedMeshes(Scene* scene);
std::size_t DropUnreferencedMaterials(Scene* scene);
std::size_t DropUnreferencedTextures(Scene* scene);
void SortSceneCollections(Scene* scene);
Result<Scene> MergeScenes(const Scene& left, const Scene& right, std::string merged_name);

}  // namespace bse
