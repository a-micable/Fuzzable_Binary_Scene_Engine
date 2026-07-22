#pragma once

#include "bse/scene.hpp"
#include "bse/status.hpp"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace bse {

enum class OptimizationPass {
  kRemoveDegenerateTriangles,
  kRemoveUnusedResources,
  kMergeDuplicateTextures,
  kMergeDuplicateMaterials,
  kCompactObjectIds,
  kStripEmptyAnimations,
  kStripEmptySkeletons,
  kSortCollections
};

struct OptimizationOptions {
  std::vector<OptimizationPass> passes;
  float position_epsilon = 0.00001F;
  bool validate_after_pass = true;
};

struct OptimizationAction {
  OptimizationPass pass = OptimizationPass::kSortCollections;
  std::string subject;
  std::string detail;
  std::size_t count = 0;
};

struct OptimizationReport {
  std::vector<OptimizationAction> actions;
  std::size_t removed_triangles = 0;
  std::size_t removed_meshes = 0;
  std::size_t removed_materials = 0;
  std::size_t removed_textures = 0;
  std::size_t removed_animations = 0;
  std::size_t removed_skeletons = 0;
  std::size_t merged_materials = 0;
  std::size_t merged_textures = 0;
  std::size_t remapped_ids = 0;
};

struct OptimizedScene {
  Scene scene;
  OptimizationReport report;
};

std::vector<OptimizationPass> DefaultOptimizationPasses();
std::string OptimizationPassName(OptimizationPass pass);
Result<OptimizedScene> OptimizeScene(const Scene& scene,
                                     const OptimizationOptions& options = OptimizationOptions{});
std::string FormatOptimizationReport(const OptimizationReport& report);

}  // namespace bse
