#pragma once

#include "bse/scene.hpp"

#include <string>
#include <vector>

namespace bse {

enum class DiffSeverity {
  kInfo,
  kWarning,
  kBreaking
};

struct DiffEntry {
  DiffSeverity severity = DiffSeverity::kInfo;
  std::string path;
  std::string message;
  std::string left;
  std::string right;
};

struct SceneDiff {
  std::vector<DiffEntry> entries;
  bool empty() const { return entries.empty(); }
};

SceneDiff DiffScenes(const Scene& left, const Scene& right);
std::string FormatDiff(const SceneDiff& diff);
std::string DiffSeverityName(DiffSeverity severity);

}  // namespace bse
