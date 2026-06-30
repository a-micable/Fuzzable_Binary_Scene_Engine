#pragma once

#include "bse/asset_catalog.hpp"
#include "bse/scene_diff.hpp"
#include "bse/scene_lint.hpp"
#include "bse/scene_stats.hpp"

#include <string>

namespace bse {

struct ReportOptions {
  bool include_stats = true;
  bool include_assets = true;
  bool include_lint = true;
  bool include_bounds = true;
};

std::string WriteMarkdownReport(const Scene& scene, const ReportOptions& options = ReportOptions{});
std::string WriteJsonReport(const Scene& scene, const ReportOptions& options = ReportOptions{});
std::string WriteDiffMarkdownReport(const SceneDiff& diff);

}  // namespace bse
