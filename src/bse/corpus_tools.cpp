#include "bse/corpus_tools.hpp"

#include "bse/mesh_tools.hpp"
#include "bse/scene_builder.hpp"
#include "bse/serializer.hpp"
#include "bse/validator.hpp"

#include <sstream>

namespace bse {

std::vector<CorpusCase> GenerateSceneCorpus(const CorpusOptions& options) {
  std::vector<CorpusCase> cases;
  cases.push_back({"grid", MakeGridScene(options.grid_size, options.grid_size)});
  cases.push_back({"primitive-showcase", MakePrimitiveShowcaseScene()});
  cases.push_back({"nested-hierarchy", MakeNestedHierarchyScene(12)});
  if (options.include_animation_cases) {
    cases.push_back({"animated-stress", MakeAnimatedStressScene(6, 12)});
  }
  if (options.include_invalid_reference_case) {
    auto invalid = MakeInvalidReferenceScene();
    if (invalid.ok()) {
      cases.push_back({"invalid-reference", invalid.value()});
    }
  }
  return cases;
}

Result<std::vector<std::uint8_t>> SerializeCorpusCase(const CorpusCase& corpus_case) {
  return SerializeScene(corpus_case.scene);
}

Result<std::vector<std::vector<std::uint8_t>>> SerializeCorpus(const std::vector<CorpusCase>& cases) {
  std::vector<std::vector<std::uint8_t>> out;
  out.reserve(cases.size());
  for (const auto& corpus_case : cases) {
    auto bytes = SerializeCorpusCase(corpus_case);
    if (!bytes.ok()) {
      return bytes.status();
    }
    out.push_back(bytes.value());
  }
  return out;
}

Result<Scene> MakeInvalidReferenceScene() {
  Scene scene = MakePrimitiveShowcaseScene();
  if (scene.nodes.empty()) {
    return Status::Error(ErrorCode::kValidationFailed, "cannot build invalid scene from empty base");
  }
  scene.nodes.front().mesh = 999999;
  return scene;
}

Scene MakeNestedHierarchyScene(std::uint32_t depth) {
  SceneBuilder builder({"nested-hierarchy", true, true, true});
  ObjectId parent = 1;
  const auto material = builder.AddMaterial({"nested_matte", {0.7F, 0.7F, 0.8F}, 0.8F, 0.0F});
  for (std::uint32_t i = 0; i < depth; ++i) {
    MeshBuildOptions options;
    options.name = "level_" + std::to_string(i) + "_box";
    options.material = material;
    auto node = builder.AddMeshNode("level_" + std::to_string(i), MakeBoxMesh(0.2F, 0.2F, 0.2F, options),
                                    parent);
    Transform transform;
    transform.translation = {0.25F, 0.1F, 0.0F};
    builder.SetNodeTransform(node, transform);
    parent = node;
  }
  auto scene = builder.Build();
  return scene.ok() ? scene.value() : Scene{};
}

Scene MakeAnimatedStressScene(std::uint32_t channels, std::uint32_t keys_per_channel) {
  SceneBuilder builder({"animated-stress", true, true, true});
  const auto material = builder.AddMaterial({"stress_matte", {0.3F, 0.6F, 0.9F}, 0.5F, 0.0F});
  std::vector<ObjectId> nodes;
  for (std::uint32_t i = 0; i < channels; ++i) {
    MeshBuildOptions options;
    options.name = "animated_box_" + std::to_string(i);
    options.material = material;
    auto node = builder.AddMeshNode("animated_node_" + std::to_string(i),
                                    MakeBoxMesh(0.25F, 0.25F, 0.25F, options));
    Transform transform;
    transform.translation = {static_cast<float>(i) * 0.4F, 0.0F, 0.0F};
    builder.SetNodeTransform(node, transform);
    nodes.push_back(node);
  }
  auto scene_result = builder.Build();
  Scene scene = scene_result.ok() ? scene_result.value() : Scene{};
  Animation animation;
  animation.id = 9000;
  animation.name = "stress_animation";
  animation.duration_seconds = 2.0F;
  for (std::uint32_t channel_index = 0; channel_index < nodes.size(); ++channel_index) {
    AnimationChannel channel;
    channel.target_node = nodes[channel_index];
    for (std::uint32_t key_index = 0; key_index < keys_per_channel; ++key_index) {
      const float t = keys_per_channel <= 1
                          ? 0.0F
                          : (static_cast<float>(key_index) / static_cast<float>(keys_per_channel - 1U)) *
                                animation.duration_seconds;
      Transform transform;
      transform.translation = {static_cast<float>(channel_index) * 0.4F, t, 0.0F};
      channel.keys.push_back({t, transform});
    }
    animation.channels.push_back(channel);
  }
  scene.animations.push_back(animation);
  return scene;
}

std::string CorpusManifest(const std::vector<CorpusCase>& cases) {
  std::ostringstream out;
  for (const auto& corpus_case : cases) {
    const auto stats = ComputeSceneStats(corpus_case.scene);
    out << corpus_case.name << " nodes=" << stats.node_count << " meshes=" << stats.geometry.mesh_count
        << " vertices=" << stats.geometry.vertex_count << " animations="
        << stats.animation.animation_count << "\n";
  }
  return out.str();
}

}  // namespace bse
