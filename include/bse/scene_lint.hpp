#pragma once

#include "bse/scene.hpp"

#include <string>
#include <vector>

namespace bse {

enum class LintLevel {
  kNote,
  kWarning,
  kError
};

struct LintFinding {
  LintLevel level = LintLevel::kNote;
  std::string rule;
  std::string subject;
  std::string message;
};

struct LintOptions {
  bool require_named_objects = true;
  bool require_mesh_normals = true;
  bool warn_unreferenced_resources = true;
  bool warn_large_textures = true;
  std::uint32_t large_texture_threshold = 4096;
};

std::vector<LintFinding> LintScene(const Scene& scene,
                                   const LintOptions& options = LintOptions{});
std::string FormatLintFindings(const std::vector<LintFinding>& findings);
std::string LintLevelName(LintLevel level);
bool HasLintErrors(const std::vector<LintFinding>& findings);

}  // namespace bse
