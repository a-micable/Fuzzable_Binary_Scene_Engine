#include "bse/scene_report.hpp"

#include <sstream>

namespace bse {

namespace {

std::string EscapeJson(const std::string& value) {
  std::string out;
  for (char c : value) {
    switch (c) {
      case '\\':
        out += "\\\\";
        break;
      case '"':
        out += "\\\"";
        break;
      case '\n':
        out += "\\n";
        break;
      default:
        out.push_back(c);
        break;
    }
  }
  return out;
}

void JsonField(std::ostream& out, const std::string& name, std::size_t value, bool comma = true) {
  out << "    \"" << name << "\": " << value;
  if (comma) out << ",";
  out << "\n";
}

}  // namespace

std::string WriteMarkdownReport(const Scene& scene, const ReportOptions& options) {
  std::ostringstream out;
  out << "# Scene Report: " << scene.name << "\n\n";
  if (options.include_stats) {
    const auto stats = ComputeSceneStats(scene);
    out << "## Statistics\n\n";
    out << "- Nodes: " << stats.node_count << "\n";
    out << "- Meshes: " << stats.geometry.mesh_count << "\n";
    out << "- Vertices: " << stats.geometry.vertex_count << "\n";
    out << "- Indices: " << stats.geometry.index_count << "\n";
    out << "- Triangles: " << stats.geometry.triangle_count << "\n";
    out << "- Materials: " << stats.resources.material_count << "\n";
    out << "- Textures: " << stats.resources.texture_count << "\n";
    out << "- Animations: " << stats.animation.animation_count << "\n\n";
  }
  if (options.include_bounds) {
    auto bounds = ComputeSceneBounds(scene);
    out << "## Bounds\n\n";
    if (bounds.valid) {
      out << "- Min: " << bounds.min.x << ", " << bounds.min.y << ", " << bounds.min.z << "\n";
      out << "- Max: " << bounds.max.x << ", " << bounds.max.y << ", " << bounds.max.z << "\n\n";
    } else {
      out << "No mesh bounds available.\n\n";
    }
  }
  if (options.include_assets) {
    auto catalog = BuildAssetCatalog(scene);
    out << "## Assets\n\n";
    for (const auto& record : catalog.records) {
      out << "- " << AssetKindName(record.kind) << " `" << record.id << "` " << record.name;
      if (!record.uri.empty()) {
        out << " (" << record.uri << ")";
      }
      out << ", references: " << record.referenced_by.size() << "\n";
    }
    out << "\n";
  }
  if (options.include_lint) {
    auto findings = LintScene(scene);
    out << "## Lint\n\n";
    if (findings.empty()) {
      out << "No lint findings.\n";
    } else {
      for (const auto& finding : findings) {
        out << "- " << LintLevelName(finding.level) << " " << finding.rule << " "
            << finding.subject << ": " << finding.message << "\n";
      }
    }
  }
  return out.str();
}

std::string WriteJsonReport(const Scene& scene, const ReportOptions& options) {
  const auto stats = ComputeSceneStats(scene);
  std::ostringstream out;
  out << "{\n";
  out << "  \"scene\": \"" << EscapeJson(scene.name) << "\",\n";
  if (options.include_stats) {
    out << "  \"stats\": {\n";
    JsonField(out, "nodes", stats.node_count);
    JsonField(out, "meshes", stats.geometry.mesh_count);
    JsonField(out, "vertices", stats.geometry.vertex_count);
    JsonField(out, "indices", stats.geometry.index_count);
    JsonField(out, "triangles", stats.geometry.triangle_count);
    JsonField(out, "materials", stats.resources.material_count);
    JsonField(out, "textures", stats.resources.texture_count);
    JsonField(out, "animations", stats.animation.animation_count, false);
    out << "  },\n";
  }
  if (options.include_bounds) {
    auto bounds = ComputeSceneBounds(scene);
    out << "  \"bounds\": ";
    if (bounds.valid) {
      out << "{\"min\": [" << bounds.min.x << ", " << bounds.min.y << ", " << bounds.min.z
          << "], \"max\": [" << bounds.max.x << ", " << bounds.max.y << ", " << bounds.max.z
          << "]},\n";
    } else {
      out << "null,\n";
    }
  }
  if (options.include_assets) {
    auto catalog = BuildAssetCatalog(scene);
    out << "  \"assets\": [\n";
    for (std::size_t i = 0; i < catalog.records.size(); ++i) {
      const auto& record = catalog.records[i];
      out << "    {\"kind\": \"" << AssetKindName(record.kind) << "\", \"id\": " << record.id
          << ", \"name\": \"" << EscapeJson(record.name) << "\", \"uri\": \""
          << EscapeJson(record.uri) << "\", \"references\": " << record.referenced_by.size()
          << "}";
      if (i + 1U != catalog.records.size()) out << ",";
      out << "\n";
    }
    out << "  ],\n";
  }
  if (options.include_lint) {
    auto findings = LintScene(scene);
    out << "  \"lint\": [\n";
    for (std::size_t i = 0; i < findings.size(); ++i) {
      const auto& finding = findings[i];
      out << "    {\"level\": \"" << LintLevelName(finding.level) << "\", \"rule\": \""
          << EscapeJson(finding.rule) << "\", \"subject\": \"" << EscapeJson(finding.subject)
          << "\", \"message\": \"" << EscapeJson(finding.message) << "\"}";
      if (i + 1U != findings.size()) out << ",";
      out << "\n";
    }
    out << "  ]\n";
  } else {
    out << "  \"lint\": []\n";
  }
  out << "}\n";
  return out.str();
}

std::string WriteDiffMarkdownReport(const SceneDiff& diff) {
  std::ostringstream out;
  out << "# Scene Diff\n\n";
  if (diff.entries.empty()) {
    out << "No differences.\n";
    return out.str();
  }
  for (const auto& entry : diff.entries) {
    out << "- **" << DiffSeverityName(entry.severity) << "** `" << entry.path << "`: "
        << entry.message;
    if (!entry.left.empty() || !entry.right.empty()) {
      out << " (`" << entry.left << "` -> `" << entry.right << "`)";
    }
    out << "\n";
  }
  return out.str();
}

}  // namespace bse
