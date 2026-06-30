#include "bse/scene.hpp"

#include <utility>

namespace bse {

Scene MakeMinimalScene(std::string name) {
  Scene scene;
  scene.name = std::move(name);
  scene.metadata.emplace("generator", "Binary Scene Engine");
  Node root;
  root.id = 1;
  root.name = "root";
  scene.nodes.push_back(std::move(root));
  return scene;
}

}  // namespace bse
