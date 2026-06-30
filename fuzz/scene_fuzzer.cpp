#include "bse/parser.hpp"
#include "bse/scene_graph.hpp"

#include <cstddef>
#include <cstdint>

extern "C" int LLVMFuzzerTestOneInput(const std::uint8_t* data, std::size_t size) {
  auto scene = bse::ParseScene(data, size);
  if (scene.ok()) {
    auto graph = bse::BuildSceneGraph(scene.value());
    auto traversal = bse::TraverseDepthFirst(graph);
    auto formatted = bse::FormatGraphTraversal(traversal);
    (void)formatted;
  }
  (void)scene;
  return 0;
}
