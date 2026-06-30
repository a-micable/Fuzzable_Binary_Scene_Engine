#include "bse/scene.hpp"
#include "bse/validator.hpp"

#include "test_support.hpp"

namespace {

void AcceptsMinimalScene() {
  bse::DiagnosticSink diagnostics;
  auto status = bse::ValidateScene(bse::MakeMinimalScene("valid"), &diagnostics);
  BSE_EXPECT_TRUE(status.ok());
  BSE_EXPECT_TRUE(!diagnostics.has_errors());
}

void RejectsDuplicateIds() {
  auto scene = bse::MakeMinimalScene("invalid");
  scene.nodes.push_back(scene.nodes.front());
  bse::DiagnosticSink diagnostics;
  auto status = bse::ValidateScene(scene, &diagnostics);
  BSE_EXPECT_TRUE(!status.ok());
  BSE_EXPECT_TRUE(diagnostics.has_errors());
}

void RejectsMissingMeshReference() {
  auto scene = bse::MakeMinimalScene("invalid");
  scene.nodes.front().mesh = 999;
  bse::DiagnosticSink diagnostics;
  auto status = bse::ValidateScene(scene, &diagnostics);
  BSE_EXPECT_TRUE(!status.ok());
  BSE_EXPECT_TRUE(diagnostics.has_errors());
}

void RejectsOutOfRangeMeshIndex() {
  auto scene = bse::MakeMinimalScene("invalid");
  bse::Mesh mesh;
  mesh.id = 10;
  mesh.vertices.push_back({{0.0F, 0.0F, 0.0F}, {}, {}});
  mesh.indices = {1};
  scene.meshes.push_back(mesh);
  scene.nodes.front().mesh = 10;

  bse::DiagnosticSink diagnostics;
  auto status = bse::ValidateScene(scene, &diagnostics);
  BSE_EXPECT_TRUE(!status.ok());
  BSE_EXPECT_TRUE(diagnostics.has_errors());
}

void RejectsInvalidAnimationTarget() {
  auto scene = bse::MakeMinimalScene("invalid");
  bse::Animation animation;
  animation.id = 20;
  animation.duration_seconds = 1.0F;
  bse::AnimationChannel channel;
  channel.target_node = 999;
  channel.keys.push_back({0.0F, {}});
  animation.channels.push_back(channel);
  scene.animations.push_back(animation);

  bse::DiagnosticSink diagnostics;
  auto status = bse::ValidateScene(scene, &diagnostics);
  BSE_EXPECT_TRUE(!status.ok());
  BSE_EXPECT_TRUE(diagnostics.has_errors());
}

}  // namespace

int main() {
  AcceptsMinimalScene();
  RejectsDuplicateIds();
  RejectsMissingMeshReference();
  RejectsOutOfRangeMeshIndex();
  RejectsInvalidAnimationTarget();
  return 0;
}
