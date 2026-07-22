#pragma once

#include "bse/scene.hpp"
#include "bse/status.hpp"

#include <string>
#include <vector>

namespace bse {

struct TextSceneOptions {
  bool include_comments = true;
  bool compact_vectors = false;
};

Result<Scene> ParseTextScene(const std::string& text);
Result<std::string> WriteTextScene(const Scene& scene,
                                   const TextSceneOptions& options = TextSceneOptions{});
Result<Scene> ParseTextSceneLines(const std::vector<std::string>& lines);
std::vector<std::string> SplitTextSceneLines(const std::string& text);

}  // namespace bse
