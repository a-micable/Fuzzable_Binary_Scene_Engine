#include "bse/scene_lint.hpp"

#include "bse/scene_stats.hpp"

#include <cmath>
#include <sstream>
#include <unordered_set>

namespace bse {

namespace {

void Add(std::vector<LintFinding>* findings, LintLevel level, std::string rule,
         std::string subject, std::string message) {
  findings->push_back({level, std::move(rule), std::move(subject), std::move(message)});
}

bool EmptyName(const std::string& name) {
  return name.empty() || name == "Untitled";
}

bool NormalLooksEmpty(const Vec3& normal) {
  const float length_squared = normal.x * normal.x + normal.y * normal.y + normal.z * normal.z;
  return length_squared < 0.000001F;
}

std::unordered_set<ObjectId> ToSet(const std::vector<ObjectId>& ids) {
  return std::unordered_set<ObjectId>(ids.begin(), ids.end());
}

}  // namespace

std::string LintLevelName(LintLevel level) {
  switch (level) {
    case LintLevel::kNote:
      return "note";
    case LintLevel::kWarning:
      return "warning";
    case LintLevel::kError:
      return "error";
  }
  return "unknown";
}

std::vector<LintFinding> LintScene(const Scene& scene, const LintOptions& options) {
  std::vector<LintFinding> findings;
  if (EmptyName(scene.name)) {
    Add(&findings, LintLevel::kWarning, "scene-name", "scene",
        "scene should have a descriptive name");
  }
  if (scene.nodes.empty()) {
    Add(&findings, LintLevel::kError, "scene-root", "scene", "scene has no nodes");
  }
  if (options.require_named_objects) {
    for (const auto& node : scene.nodes) {
      if (EmptyName(node.name)) {
        Add(&findings, LintLevel::kWarning, "object-name", std::to_string(node.id),
            "node should have a descriptive name");
      }
    }
    for (const auto& mesh : scene.meshes) {
      if (EmptyName(mesh.name)) {
        Add(&findings, LintLevel::kWarning, "object-name", std::to_string(mesh.id),
            "mesh should have a descriptive name");
      }
    }
    for (const auto& material : scene.materials) {
      if (EmptyName(material.name)) {
        Add(&findings, LintLevel::kWarning, "object-name", std::to_string(material.id),
            "material should have a descriptive name");
      }
    }
  }

  if (options.require_mesh_normals) {
    for (const auto& mesh : scene.meshes) {
      for (const auto& vertex : mesh.vertices) {
        if (NormalLooksEmpty(vertex.normal)) {
          Add(&findings, LintLevel::kWarning, "mesh-normal", mesh.name,
              "mesh contains a vertex with an empty normal");
          break;
        }
      }
    }
  }

  if (options.warn_unreferenced_resources) {
    const auto referenced_materials = ToSet(FindReferencedMaterials(scene));
    const auto referenced_textures = ToSet(FindReferencedTextures(scene));
    for (const auto& material : scene.materials) {
      if (referenced_materials.find(material.id) == referenced_materials.end()) {
        Add(&findings, LintLevel::kNote, "unused-material", material.name,
            "material is not referenced by any mesh");
      }
    }
    for (const auto& texture : scene.textures) {
      if (referenced_textures.find(texture.id) == referenced_textures.end()) {
        Add(&findings, LintLevel::kNote, "unused-texture", texture.name,
            "texture is not referenced by any material");
      }
    }
  }

  if (options.warn_large_textures) {
    for (const auto& texture : scene.textures) {
      if (texture.width > options.large_texture_threshold ||
          texture.height > options.large_texture_threshold) {
        Add(&findings, LintLevel::kWarning, "large-texture", texture.name,
            "texture dimensions exceed the configured threshold");
      }
    }
  }

  for (const auto& issue : AuditSceneStructure(scene)) {
    Add(&findings, LintLevel::kError, issue.code, issue.subject, issue.message);
  }
  return findings;
}

std::string FormatLintFindings(const std::vector<LintFinding>& findings) {
  std::ostringstream out;
  for (const auto& finding : findings) {
    out << LintLevelName(finding.level) << " " << finding.rule << " " << finding.subject << ": "
        << finding.message << "\n";
  }
  return out.str();
}

bool HasLintErrors(const std::vector<LintFinding>& findings) {
  for (const auto& finding : findings) {
    if (finding.level == LintLevel::kError) {
      return true;
    }
  }
  return false;
}

}  // namespace bse
