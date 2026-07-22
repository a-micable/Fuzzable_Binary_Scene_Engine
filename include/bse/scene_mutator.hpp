#pragma once

#include "bse/scene.hpp"
#include "bse/status.hpp"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace bse {

enum class MutationKind {
  kRenameObjects,
  kPerturbTransforms,
  kPerturbGeometry,
  kDuplicateMesh,
  kDuplicateSubtree,
  kDropLeafNodes,
  kReassignMaterials,
  kExpandAnimationKeys,
  kInjectDanglingReferences,
  kQuantizeGeometry
};

struct MutationOptions {
  std::uint64_t seed = 0xB5E2026ULL;
  std::size_t max_new_nodes = 32;
  std::size_t max_new_meshes = 16;
  std::size_t max_new_keys_per_channel = 16;
  float transform_jitter = 0.25F;
  float vertex_jitter = 0.05F;
  float quantize_step = 0.001F;
  bool preserve_validity = true;
};

struct MutationStep {
  MutationKind kind = MutationKind::kRenameObjects;
  std::string subject;
  std::string note;
};

struct MutationReport {
  std::uint64_t seed = 0;
  std::vector<MutationStep> steps;
  std::size_t nodes_added = 0;
  std::size_t nodes_removed = 0;
  std::size_t meshes_added = 0;
  std::size_t vertices_changed = 0;
  std::size_t animation_keys_added = 0;
};

struct MutationRecipe {
  std::string name;
  std::vector<MutationKind> steps;
};

struct MutatedScene {
  std::string name;
  Scene scene;
  MutationReport report;
};

std::string MutationKindName(MutationKind kind);
std::vector<MutationRecipe> DefaultMutationRecipes();
Result<MutatedScene> MutateScene(const Scene& source, const MutationRecipe& recipe,
                                 const MutationOptions& options = MutationOptions{});
std::vector<MutatedScene> GenerateMutationSuite(const Scene& source,
                                                const MutationOptions& options = MutationOptions{});
std::string FormatMutationReport(const MutationReport& report);

}  // namespace bse
