#include "bse/scene_manifest.hpp"

#include <algorithm>
#include <sstream>
#include <unordered_set>

namespace bse {

namespace {

void AddResource(SceneManifest* manifest, ManifestResourceKind kind, ObjectId id,
                 std::string name, std::size_t payload_count) {
  manifest->resources.push_back({kind, id, std::move(name), payload_count});
}

void AddReference(SceneManifest* manifest, ManifestResourceKind from_kind, ObjectId from,
                  ManifestResourceKind to_kind, ObjectId to, std::string label,
                  const std::unordered_set<ObjectId>& ids) {
  if (to == kInvalidObjectId) return;
  manifest->references.push_back({from_kind, from, to_kind, to, std::move(label),
                                  ids.find(to) != ids.end()});
}

std::unordered_set<ObjectId> IdsFor(const SceneManifest& manifest, ManifestResourceKind kind) {
  std::unordered_set<ObjectId> ids;
  for (const auto& resource : manifest.resources) {
    if (resource.kind == kind) ids.insert(resource.id);
  }
  return ids;
}

void SortManifest(SceneManifest* manifest) {
  std::sort(manifest->resources.begin(), manifest->resources.end(),
            [](const ManifestResource& left, const ManifestResource& right) {
              if (left.kind != right.kind) return static_cast<int>(left.kind) < static_cast<int>(right.kind);
              return left.id < right.id;
            });
  std::sort(manifest->references.begin(), manifest->references.end(),
            [](const ManifestReference& left, const ManifestReference& right) {
              if (left.from_kind != right.from_kind) {
                return static_cast<int>(left.from_kind) < static_cast<int>(right.from_kind);
              }
              if (left.from != right.from) return left.from < right.from;
              return left.label < right.label;
            });
}

}  // namespace

SceneManifest BuildSceneManifest(const Scene& scene) {
  SceneManifest manifest;
  manifest.scene_name = scene.name;
  for (const auto& node : scene.nodes) AddResource(&manifest, ManifestResourceKind::kNode, node.id, node.name, node.children.size());
  for (const auto& mesh : scene.meshes) AddResource(&manifest, ManifestResourceKind::kMesh, mesh.id, mesh.name, mesh.vertices.size());
  for (const auto& material : scene.materials) AddResource(&manifest, ManifestResourceKind::kMaterial, material.id, material.name, 1);
  for (const auto& texture : scene.textures) AddResource(&manifest, ManifestResourceKind::kTexture, texture.id, texture.name, texture.width * texture.height);
  for (const auto& camera : scene.cameras) AddResource(&manifest, ManifestResourceKind::kCamera, camera.id, camera.name, 1);
  for (const auto& light : scene.lights) AddResource(&manifest, ManifestResourceKind::kLight, light.id, light.name, 1);
  for (const auto& skeleton : scene.skeletons) AddResource(&manifest, ManifestResourceKind::kSkeleton, skeleton.id, skeleton.name, skeleton.joints.size());
  for (const auto& animation : scene.animations) AddResource(&manifest, ManifestResourceKind::kAnimation, animation.id, animation.name, animation.channels.size());

  const auto node_ids = IdsFor(manifest, ManifestResourceKind::kNode);
  const auto mesh_ids = IdsFor(manifest, ManifestResourceKind::kMesh);
  const auto material_ids = IdsFor(manifest, ManifestResourceKind::kMaterial);
  const auto texture_ids = IdsFor(manifest, ManifestResourceKind::kTexture);
  const auto camera_ids = IdsFor(manifest, ManifestResourceKind::kCamera);
  const auto light_ids = IdsFor(manifest, ManifestResourceKind::kLight);
  for (const auto& node : scene.nodes) {
    AddReference(&manifest, ManifestResourceKind::kNode, node.id, ManifestResourceKind::kNode,
                 node.parent, "parent", node_ids);
    AddReference(&manifest, ManifestResourceKind::kNode, node.id, ManifestResourceKind::kMesh,
                 node.mesh, "mesh", mesh_ids);
    AddReference(&manifest, ManifestResourceKind::kNode, node.id, ManifestResourceKind::kCamera,
                 node.camera, "camera", camera_ids);
    AddReference(&manifest, ManifestResourceKind::kNode, node.id, ManifestResourceKind::kLight,
                 node.light, "light", light_ids);
  }
  for (const auto& mesh : scene.meshes) {
    AddReference(&manifest, ManifestResourceKind::kMesh, mesh.id, ManifestResourceKind::kMaterial,
                 mesh.material, "material", material_ids);
  }
  for (const auto& material : scene.materials) {
    AddReference(&manifest, ManifestResourceKind::kMaterial, material.id,
                 ManifestResourceKind::kTexture, material.base_color_texture, "base_color_texture",
                 texture_ids);
  }
  for (const auto& animation : scene.animations) {
    for (const auto& channel : animation.channels) {
      AddReference(&manifest, ManifestResourceKind::kAnimation, animation.id,
                   ManifestResourceKind::kNode, channel.target_node, "target_node", node_ids);
    }
  }
  SortManifest(&manifest);
  return manifest;
}

std::vector<ManifestResource> FindManifestResources(const SceneManifest& manifest,
                                                    ManifestResourceKind kind) {
  std::vector<ManifestResource> resources;
  for (const auto& resource : manifest.resources) {
    if (resource.kind == kind) resources.push_back(resource);
  }
  return resources;
}

std::vector<ManifestReference> FindUnresolvedReferences(const SceneManifest& manifest) {
  std::vector<ManifestReference> references;
  for (const auto& reference : manifest.references) {
    if (!reference.resolved) references.push_back(reference);
  }
  return references;
}

std::vector<ManifestReference> IncomingReferences(const SceneManifest& manifest,
                                                  ManifestResourceKind kind, ObjectId id) {
  std::vector<ManifestReference> references;
  for (const auto& reference : manifest.references) {
    if (reference.to_kind == kind && reference.to == id) references.push_back(reference);
  }
  return references;
}

std::vector<ManifestReference> OutgoingReferences(const SceneManifest& manifest,
                                                  ManifestResourceKind kind, ObjectId id) {
  std::vector<ManifestReference> references;
  for (const auto& reference : manifest.references) {
    if (reference.from_kind == kind && reference.from == id) references.push_back(reference);
  }
  return references;
}

bool ManifestContainsResource(const SceneManifest& manifest, ManifestResourceKind kind,
                              ObjectId id) {
  for (const auto& resource : manifest.resources) {
    if (resource.kind == kind && resource.id == id) return true;
  }
  return false;
}

bool ManifestHasResources(const SceneManifest& manifest) {
  return !manifest.resources.empty();
}

bool ManifestHasReferences(const SceneManifest& manifest) {
  return !manifest.references.empty();
}

bool ManifestHasUnresolvedReferences(const SceneManifest& manifest) {
  for (const auto& reference : manifest.references) {
    if (!reference.resolved) return true;
  }
  return false;
}

std::size_t CountResolvedReferences(const SceneManifest& manifest) {
  std::size_t count = 0;
  for (const auto& reference : manifest.references) {
    if (reference.resolved) ++count;
  }
  return count;
}

std::size_t CountUnresolvedReferences(const SceneManifest& manifest) {
  std::size_t count = 0;
  for (const auto& reference : manifest.references) {
    if (!reference.resolved) ++count;
  }
  return count;
}

std::vector<ManifestResource> FindUnreferencedResources(const SceneManifest& manifest,
                                                        ManifestResourceKind kind) {
  std::vector<ManifestResource> resources;
  for (const auto& resource : manifest.resources) {
    if (resource.kind != kind) continue;
    if (IncomingReferences(manifest, kind, resource.id).empty()) resources.push_back(resource);
  }
  return resources;
}

std::vector<ManifestResourceUsage> BuildManifestUsage(const SceneManifest& manifest) {
  std::vector<ManifestResourceUsage> usage;
  for (const auto& resource : manifest.resources) {
    ManifestResourceUsage item;
    item.kind = resource.kind;
    item.id = resource.id;
    item.name = resource.name;
    item.incoming = IncomingReferences(manifest, resource.kind, resource.id).size();
    item.outgoing = OutgoingReferences(manifest, resource.kind, resource.id).size();
    usage.push_back(item);
  }
  for (const auto& reference : manifest.references) {
    if (reference.resolved) continue;
    ManifestResourceUsage missing;
    missing.kind = reference.to_kind;
    missing.id = reference.to;
    missing.name = "<missing>";
    missing.incoming = 1;
    missing.unresolved = true;
    usage.push_back(missing);
  }
  std::sort(usage.begin(), usage.end(), [](const ManifestResourceUsage& left,
                                           const ManifestResourceUsage& right) {
    if (left.kind != right.kind) return static_cast<int>(left.kind) < static_cast<int>(right.kind);
    if (left.id != right.id) return left.id < right.id;
    return left.name < right.name;
  });
  return usage;
}

std::vector<ManifestResourceUsage> FindUnusedManifestResources(const SceneManifest& manifest) {
  std::vector<ManifestResourceUsage> unused;
  for (const auto& item : BuildManifestUsage(manifest)) {
    if (!item.unresolved && item.incoming == 0 && item.kind != ManifestResourceKind::kNode) {
      unused.push_back(item);
    }
  }
  return unused;
}

std::size_t CountManifestResources(const SceneManifest& manifest, ManifestResourceKind kind) {
  std::size_t count = 0;
  for (const auto& resource : manifest.resources) {
    if (resource.kind == kind) ++count;
  }
  return count;
}

ManifestSummary SummarizeManifest(const SceneManifest& manifest) {
  ManifestSummary summary;
  summary.resources = manifest.resources.size();
  summary.references = manifest.references.size();
  summary.unresolved_references = FindUnresolvedReferences(manifest).size();
  summary.nodes = CountManifestResources(manifest, ManifestResourceKind::kNode);
  summary.meshes = CountManifestResources(manifest, ManifestResourceKind::kMesh);
  summary.materials = CountManifestResources(manifest, ManifestResourceKind::kMaterial);
  summary.textures = CountManifestResources(manifest, ManifestResourceKind::kTexture);
  summary.animations = CountManifestResources(manifest, ManifestResourceKind::kAnimation);
  return summary;
}

std::string ManifestResourceKindName(ManifestResourceKind kind) {
  switch (kind) {
    case ManifestResourceKind::kNode:
      return "node";
    case ManifestResourceKind::kMesh:
      return "mesh";
    case ManifestResourceKind::kMaterial:
      return "material";
    case ManifestResourceKind::kTexture:
      return "texture";
    case ManifestResourceKind::kCamera:
      return "camera";
    case ManifestResourceKind::kLight:
      return "light";
    case ManifestResourceKind::kSkeleton:
      return "skeleton";
    case ManifestResourceKind::kAnimation:
      return "animation";
  }
  return "unknown";
}

std::string FormatManifestSummary(const ManifestSummary& summary) {
  std::ostringstream out;
  out << "resources=" << summary.resources << "\n";
  out << "references=" << summary.references << "\n";
  out << "unresolved_references=" << summary.unresolved_references << "\n";
  out << "nodes=" << summary.nodes << "\n";
  out << "meshes=" << summary.meshes << "\n";
  out << "materials=" << summary.materials << "\n";
  out << "textures=" << summary.textures << "\n";
  out << "animations=" << summary.animations << "\n";
  return out.str();
}

std::string FormatManifestUsage(const std::vector<ManifestResourceUsage>& usage) {
  std::ostringstream out;
  out << "usage=" << usage.size() << "\n";
  for (const auto& item : usage) {
    out << ManifestResourceKindName(item.kind) << " " << item.id << " " << item.name
        << " incoming=" << item.incoming << " outgoing=" << item.outgoing
        << " unresolved=" << (item.unresolved ? "true" : "false") << "\n";
  }
  return out.str();
}

std::string FormatSceneManifest(const SceneManifest& manifest) {
  std::ostringstream out;
  out << "scene=" << manifest.scene_name << "\n";
  out << FormatManifestSummary(SummarizeManifest(manifest));
  out << FormatManifestUsage(BuildManifestUsage(manifest));
  out << "resources=" << manifest.resources.size() << "\n";
  for (const auto& resource : manifest.resources) {
    out << ManifestResourceKindName(resource.kind) << " " << resource.id << " "
        << resource.name << " payload=" << resource.payload_count << "\n";
  }
  out << "references=" << manifest.references.size() << "\n";
  for (const auto& reference : manifest.references) {
    out << ManifestResourceKindName(reference.from_kind) << ":" << reference.from << " -> "
        << ManifestResourceKindName(reference.to_kind) << ":" << reference.to << " "
        << reference.label << " resolved=" << (reference.resolved ? "true" : "false") << "\n";
  }
  return out.str();
}

std::string WriteManifestCsv(const SceneManifest& manifest) {
  std::ostringstream out;
  out << "section,kind,id,name,label,target_kind,target_id,resolved\n";
  for (const auto& resource : manifest.resources) {
    out << "resource," << ManifestResourceKindName(resource.kind) << "," << resource.id << ","
        << resource.name << ",,,,\n";
  }
  for (const auto& reference : manifest.references) {
    out << "reference," << ManifestResourceKindName(reference.from_kind) << "," << reference.from
        << ",," << reference.label << "," << ManifestResourceKindName(reference.to_kind) << ","
        << reference.to << "," << (reference.resolved ? "true" : "false") << "\n";
  }
  return out.str();
}

std::string WriteManifestJson(const SceneManifest& manifest) {
  std::ostringstream out;
  out << "{";
  out << "\"scene\":\"" << manifest.scene_name << "\",";
  out << "\"resources\":[";
  for (std::size_t i = 0; i < manifest.resources.size(); ++i) {
    const auto& resource = manifest.resources[i];
    if (i != 0) out << ",";
    out << "{\"kind\":\"" << ManifestResourceKindName(resource.kind) << "\",";
    out << "\"id\":" << resource.id << ",";
    out << "\"name\":\"" << resource.name << "\",";
    out << "\"payload\":" << resource.payload_count << "}";
  }
  out << "],\"references\":[";
  for (std::size_t i = 0; i < manifest.references.size(); ++i) {
    const auto& reference = manifest.references[i];
    if (i != 0) out << ",";
    out << "{\"from_kind\":\"" << ManifestResourceKindName(reference.from_kind) << "\",";
    out << "\"from\":" << reference.from << ",";
    out << "\"to_kind\":\"" << ManifestResourceKindName(reference.to_kind) << "\",";
    out << "\"to\":" << reference.to << ",";
    out << "\"label\":\"" << reference.label << "\",";
    out << "\"resolved\":" << (reference.resolved ? "true" : "false") << "}";
  }
  out << "]}";
  return out.str();
}

std::string WriteManifestMarkdown(const SceneManifest& manifest) {
  std::ostringstream out;
  out << "# Scene Manifest: " << manifest.scene_name << "\n\n";
  out << "## Resources\n\n";
  out << "| Kind | Id | Name | Payload |\n";
  out << "| --- | ---: | --- | ---: |\n";
  for (const auto& resource : manifest.resources) {
    out << "| " << ManifestResourceKindName(resource.kind) << " | " << resource.id << " | "
        << resource.name << " | " << resource.payload_count << " |\n";
  }
  out << "\n## References\n\n";
  out << "| From | Label | To | Resolved |\n";
  out << "| --- | --- | --- | --- |\n";
  for (const auto& reference : manifest.references) {
    out << "| " << ManifestResourceKindName(reference.from_kind) << ":" << reference.from << " | "
        << reference.label << " | " << ManifestResourceKindName(reference.to_kind) << ":"
        << reference.to << " | " << (reference.resolved ? "yes" : "no") << " |\n";
  }
  return out.str();
}

std::string WriteManifestDot(const SceneManifest& manifest) {
  std::ostringstream out;
  out << "digraph manifest {\n";
  for (const auto& resource : manifest.resources) {
    out << "  r" << static_cast<int>(resource.kind) << "_" << resource.id << " [label=\""
        << ManifestResourceKindName(resource.kind) << ":" << resource.id << "\"];\n";
  }
  for (const auto& reference : manifest.references) {
    out << "  r" << static_cast<int>(reference.from_kind) << "_" << reference.from << " -> r"
        << static_cast<int>(reference.to_kind) << "_" << reference.to << " [label=\""
        << reference.label << "\"";
    if (!reference.resolved) out << ", style=dashed";
    out << "];\n";
  }
  out << "}\n";
  return out.str();
}

std::vector<std::string> ValidateManifestConsistency(const SceneManifest& manifest) {
  std::vector<std::string> issues;
  for (const auto& resource : manifest.resources) {
    if (resource.id == kInvalidObjectId) {
      issues.push_back("resource has invalid object id: " + resource.name);
    }
  }
  for (const auto& reference : manifest.references) {
    if (!ManifestContainsResource(manifest, reference.from_kind, reference.from)) {
      issues.push_back("reference source is missing: " +
                       ManifestResourceKindName(reference.from_kind) + ":" +
                       std::to_string(reference.from));
    }
    if (reference.resolved &&
        !ManifestContainsResource(manifest, reference.to_kind, reference.to)) {
      issues.push_back("reference marked resolved but target is missing: " +
                       ManifestResourceKindName(reference.to_kind) + ":" +
                       std::to_string(reference.to));
    }
  }
  return issues;
}

}  // namespace bse
