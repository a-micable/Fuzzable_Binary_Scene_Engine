#include "bse/validation_profile.hpp"

#include "bse/scene_stats.hpp"

#include <algorithm>
#include <sstream>
#include <unordered_map>
#include <unordered_set>

namespace bse {

namespace {

void AddFinding(ProfileReport* report, ProfileSeverity severity, std::string code,
                std::string subject, std::string message) {
  report->findings.push_back({severity, std::move(code), std::move(subject), std::move(message)});
}

std::string CountMessage(std::size_t value, std::size_t limit, const std::string& label) {
  std::ostringstream out;
  out << label << " count " << value << " exceeds profile limit " << limit;
  return out.str();
}

std::size_t AnimationChannelCount(const Scene& scene) {
  std::size_t count = 0;
  for (const auto& animation : scene.animations) count += animation.channels.size();
  return count;
}

std::size_t AnimationKeyCount(const Scene& scene) {
  std::size_t count = 0;
  for (const auto& animation : scene.animations) {
    for (const auto& channel : animation.channels) count += channel.keys.size();
  }
  return count;
}

std::unordered_map<ObjectId, const Node*> NodeMap(const Scene& scene) {
  std::unordered_map<ObjectId, const Node*> nodes;
  for (const auto& node : scene.nodes) nodes[node.id] = &node;
  return nodes;
}

std::size_t DepthOf(ObjectId id, const std::unordered_map<ObjectId, const Node*>& nodes) {
  std::unordered_set<ObjectId> seen;
  std::size_t depth = 0;
  auto cursor = nodes.find(id);
  while (cursor != nodes.end() && cursor->second->parent != kInvalidObjectId) {
    if (!seen.insert(cursor->second->id).second) return depth + 1;
    cursor = nodes.find(cursor->second->parent);
    ++depth;
  }
  return depth;
}

std::size_t MaxDepth(const Scene& scene) {
  const auto nodes = NodeMap(scene);
  std::size_t max_depth = 0;
  for (const auto& node : scene.nodes) {
    max_depth = std::max(max_depth, DepthOf(node.id, nodes));
  }
  return max_depth;
}

void CheckName(ProfileReport* report, const ValidationProfile& profile, const std::string& subject,
               const std::string& name) {
  if (name.size() > profile.max_name_bytes) {
    AddFinding(report, ProfileSeverity::kError, "name-too-long", subject,
               CountMessage(name.size(), profile.max_name_bytes, "name byte"));
  }
}

void CheckNames(ProfileReport* report, const Scene& scene, const ValidationProfile& profile) {
  CheckName(report, profile, "scene", scene.name);
  for (const auto& node : scene.nodes) CheckName(report, profile, "node:" + std::to_string(node.id), node.name);
  for (const auto& mesh : scene.meshes) CheckName(report, profile, "mesh:" + std::to_string(mesh.id), mesh.name);
  for (const auto& material : scene.materials) {
    CheckName(report, profile, "material:" + std::to_string(material.id), material.name);
  }
  for (const auto& texture : scene.textures) {
    CheckName(report, profile, "texture:" + std::to_string(texture.id), texture.name);
    CheckName(report, profile, "texture-uri:" + std::to_string(texture.id), texture.uri);
  }
  for (const auto& animation : scene.animations) {
    CheckName(report, profile, "animation:" + std::to_string(animation.id), animation.name);
  }
}

void CheckCounts(ProfileReport* report, const Scene& scene, const SceneStats& stats,
                 const ValidationProfile& profile) {
  if (stats.node_count > profile.limits.max_nodes) {
    AddFinding(report, ProfileSeverity::kError, "too-many-nodes", "scene",
               CountMessage(stats.node_count, profile.limits.max_nodes, "node"));
  }
  if (stats.geometry.vertex_count > profile.limits.max_vertices) {
    AddFinding(report, ProfileSeverity::kError, "too-many-vertices", "scene",
               CountMessage(stats.geometry.vertex_count, profile.limits.max_vertices, "vertex"));
  }
  if (stats.geometry.index_count > profile.limits.max_indices) {
    AddFinding(report, ProfileSeverity::kError, "too-many-indices", "scene",
               CountMessage(stats.geometry.index_count, profile.limits.max_indices, "index"));
  }
  if (scene.materials.size() > profile.max_materials) {
    AddFinding(report, ProfileSeverity::kError, "too-many-materials", "scene",
               CountMessage(scene.materials.size(), profile.max_materials, "material"));
  }
  if (scene.textures.size() > profile.max_textures) {
    AddFinding(report, ProfileSeverity::kError, "too-many-textures", "scene",
               CountMessage(scene.textures.size(), profile.max_textures, "texture"));
  }
  if (scene.metadata.size() > profile.max_metadata_entries) {
    AddFinding(report, ProfileSeverity::kError, "too-many-metadata-entries", "scene",
               CountMessage(scene.metadata.size(), profile.max_metadata_entries, "metadata"));
  }
  const std::size_t channel_count = AnimationChannelCount(scene);
  if (channel_count > profile.max_animation_channels) {
    AddFinding(report, ProfileSeverity::kError, "too-many-animation-channels", "scene",
               CountMessage(channel_count, profile.max_animation_channels, "animation channel"));
  }
  const std::size_t key_count = AnimationKeyCount(scene);
  if (key_count > profile.max_animation_keys) {
    AddFinding(report, ProfileSeverity::kError, "too-many-animation-keys", "scene",
               CountMessage(key_count, profile.max_animation_keys, "animation key"));
  }
}

void CheckTopology(ProfileReport* report, const Scene& scene, const SceneStats& stats,
                   const ValidationProfile& profile) {
  if (profile.require_root_node && stats.root_node_count == 0) {
    AddFinding(report, ProfileSeverity::kError, "missing-root", "scene",
               "profile requires at least one root node");
  }
  const std::size_t depth = MaxDepth(scene);
  if (depth > profile.max_depth) {
    AddFinding(report, ProfileSeverity::kError, "hierarchy-too-deep", "scene",
               CountMessage(depth, profile.max_depth, "hierarchy depth"));
  }
  for (const auto& node : scene.nodes) {
    if (node.children.size() > profile.max_children_per_node) {
      AddFinding(report, ProfileSeverity::kError, "too-many-children", "node:" + std::to_string(node.id),
                 CountMessage(node.children.size(), profile.max_children_per_node, "child"));
    }
  }
  if (profile.require_reachable_nodes) {
    for (ObjectId id : FindUnreachableNodes(scene)) {
      AddFinding(report, ProfileSeverity::kWarning, "unreachable-node", "node:" + std::to_string(id),
                 "node is not reachable from any root");
    }
  }
}

void CheckMeshes(ProfileReport* report, const Scene& scene, const ValidationProfile& profile) {
  for (const auto& mesh : scene.meshes) {
    const std::string subject = "mesh:" + std::to_string(mesh.id);
    if (!profile.allow_empty_meshes && mesh.vertices.empty()) {
      AddFinding(report, ProfileSeverity::kError, "empty-mesh", subject,
                 "profile does not allow empty mesh payloads");
    }
    if (profile.require_indexed_triangles && mesh.indices.empty()) {
      AddFinding(report, ProfileSeverity::kError, "unindexed-mesh", subject,
                 "profile requires indexed triangle meshes");
    }
    if (mesh.indices.size() % 3U != 0U && !mesh.indices.empty()) {
      AddFinding(report, ProfileSeverity::kWarning, "non-triangle-index-count", subject,
                 "index count is not divisible by three");
    }
    if (!profile.allow_missing_materials && mesh.material == kInvalidObjectId && !scene.materials.empty()) {
      AddFinding(report, ProfileSeverity::kWarning, "missing-material", subject,
                 "mesh does not reference a material");
    }
  }
}

void CheckTextures(ProfileReport* report, const Scene& scene) {
  for (const auto& texture : scene.textures) {
    if (texture.width == 0 || texture.height == 0) {
      AddFinding(report, ProfileSeverity::kWarning, "zero-sized-texture",
                 "texture:" + std::to_string(texture.id), "texture has a zero dimension");
    }
    if (texture.color_space != "linear" && texture.color_space != "srgb") {
      AddFinding(report, ProfileSeverity::kWarning, "unusual-color-space",
                 "texture:" + std::to_string(texture.id), "texture color space is not linear or srgb");
    }
  }
}

}  // namespace

ValidationProfile DesktopRuntimeProfile() {
  ValidationProfile profile;
  profile.name = "desktop-runtime";
  profile.limits.max_nodes = 250'000;
  profile.limits.max_vertices = 16'000'000;
  profile.limits.max_indices = 48'000'000;
  profile.max_depth = 256;
  profile.max_children_per_node = 8192;
  profile.max_materials = 250'000;
  profile.max_textures = 250'000;
  profile.max_animation_channels = 500'000;
  profile.max_animation_keys = 4'000'000;
  return profile;
}

ValidationProfile MobileRuntimeProfile() {
  ValidationProfile profile = DesktopRuntimeProfile();
  profile.name = "mobile-runtime";
  profile.limits.max_nodes = 50'000;
  profile.limits.max_vertices = 2'000'000;
  profile.limits.max_indices = 6'000'000;
  profile.max_depth = 96;
  profile.max_children_per_node = 1024;
  profile.max_materials = 16'000;
  profile.max_textures = 8192;
  profile.max_animation_channels = 64'000;
  profile.max_animation_keys = 512'000;
  profile.require_indexed_triangles = true;
  return profile;
}

ValidationProfile StrictAssetProfile() {
  ValidationProfile profile = MobileRuntimeProfile();
  profile.name = "strict-asset";
  profile.allow_empty_meshes = false;
  profile.allow_missing_materials = false;
  profile.max_name_bytes = 256;
  profile.max_metadata_entries = 256;
  profile.require_reachable_nodes = true;
  return profile;
}

ValidationProfile FuzzingProfile() {
  ValidationProfile profile;
  profile.name = "fuzzing";
  profile.limits.max_nodes = 20'000;
  profile.limits.max_vertices = 750'000;
  profile.limits.max_indices = 2'250'000;
  profile.max_depth = 512;
  profile.max_children_per_node = 20'000;
  profile.max_materials = 20'000;
  profile.max_textures = 20'000;
  profile.max_animation_channels = 40'000;
  profile.max_animation_keys = 1'000'000;
  profile.max_metadata_entries = 4096;
  profile.max_name_bytes = 4096;
  profile.allow_empty_meshes = true;
  profile.allow_missing_materials = true;
  return profile;
}

std::vector<ValidationProfile> BuiltInValidationProfiles() {
  return {DesktopRuntimeProfile(), MobileRuntimeProfile(), StrictAssetProfile(), FuzzingProfile()};
}

ProfileReport CheckValidationProfile(const Scene& scene, const ValidationProfile& profile) {
  ProfileReport report;
  report.profile_name = profile.name;
  const SceneStats stats = ComputeSceneStats(scene);
  Status structural = ValidateScene(scene, nullptr, profile.limits);
  if (!structural.ok()) {
    AddFinding(&report, ProfileSeverity::kError, "structural-validation", "scene",
               structural.message());
  }
  CheckCounts(&report, scene, stats, profile);
  CheckTopology(&report, scene, stats, profile);
  CheckMeshes(&report, scene, profile);
  CheckTextures(&report, scene);
  CheckNames(&report, scene, profile);
  if (report.findings.empty()) {
    AddFinding(&report, ProfileSeverity::kInfo, "profile-clean", "scene",
               "scene satisfies profile checks");
  }
  return report;
}

bool ProfileReportHasErrors(const ProfileReport& report) {
  return std::any_of(report.findings.begin(), report.findings.end(), [](const ProfileFinding& finding) {
    return finding.severity == ProfileSeverity::kError;
  });
}

std::string ProfileSeverityName(ProfileSeverity severity) {
  switch (severity) {
    case ProfileSeverity::kInfo:
      return "info";
    case ProfileSeverity::kWarning:
      return "warning";
    case ProfileSeverity::kError:
      return "error";
  }
  return "unknown";
}

std::string FormatProfileReport(const ProfileReport& report) {
  std::ostringstream out;
  out << "profile=" << report.profile_name << "\n";
  for (const auto& finding : report.findings) {
    out << ProfileSeverityName(finding.severity) << " " << finding.code << " "
        << finding.subject << ": " << finding.message << "\n";
  }
  return out.str();
}

}  // namespace bse
