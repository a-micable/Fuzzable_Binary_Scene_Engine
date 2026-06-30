#pragma once

#include "bse/scene.hpp"
#include "bse/status.hpp"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace bse {

struct CorpusCase {
  std::string name;
  Scene scene;
};

struct CorpusOptions {
  std::uint32_t grid_size = 4;
  std::uint32_t primitive_slices = 16;
  bool include_invalid_reference_case = false;
  bool include_animation_cases = true;
};

std::vector<CorpusCase> GenerateSceneCorpus(const CorpusOptions& options = CorpusOptions{});
Result<std::vector<std::uint8_t>> SerializeCorpusCase(const CorpusCase& corpus_case);
Result<std::vector<std::vector<std::uint8_t>>> SerializeCorpus(
    const std::vector<CorpusCase>& cases);
Result<Scene> MakeInvalidReferenceScene();
Scene MakeNestedHierarchyScene(std::uint32_t depth);
Scene MakeAnimatedStressScene(std::uint32_t channels, std::uint32_t keys_per_channel);
std::string CorpusManifest(const std::vector<CorpusCase>& cases);

}  // namespace bse
