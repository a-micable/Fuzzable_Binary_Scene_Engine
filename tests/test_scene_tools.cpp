#include "bse/scene_diff.hpp"
#include "bse/scene_lint.hpp"
#include "bse/scene_ops.hpp"
#include "bse/scene_query.hpp"
#include "bse/scene_stats.hpp"
#include "bse/text_scene.hpp"
#include "bse/transform.hpp"

#include "test_support.hpp"

namespace {

bse::Scene BuildToolScene() {
  bse::Scene scene = bse::MakeMinimalScene("tool-scene");
  scene.metadata["owner"] = "tools";
  scene.textures.push_back({10, "albedo", "textures/albedo.ktx2", 128, 128, "srgb"});
  scene.materials.push_back({20, "paint", {0.4F, 0.5F, 0.6F}, 0.8F, 0.1F, 10});
  bse::Mesh mesh;
  mesh.id = 30;
  mesh.name = "triangle";
  mesh.material = 20;
  mesh.vertices.push_back({{0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 1.0F}, {0.0F, 0.0F}});
  mesh.vertices.push_back({{2.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 1.0F}, {1.0F, 0.0F}});
  mesh.vertices.push_back({{0.0F, 3.0F, 0.0F}, {0.0F, 0.0F, 1.0F}, {0.0F, 1.0F}});
  mesh.indices = {0, 1, 2};
  scene.meshes.push_back(mesh);
  scene.nodes.front().mesh = 30;
  return scene;
}

void ComputesStatsAndBounds() {
  const auto scene = BuildToolScene();
  auto stats = bse::ComputeSceneStats(scene);
  BSE_EXPECT_EQ(stats.node_count, 1U);
  BSE_EXPECT_EQ(stats.geometry.vertex_count, 3U);
  BSE_EXPECT_EQ(stats.geometry.triangle_count, 1U);
  BSE_EXPECT_EQ(stats.resources.referenced_texture_count, 1U);
  auto bounds = bse::ComputeSceneBounds(scene);
  BSE_EXPECT_TRUE(bounds.valid);
  BSE_EXPECT_EQ(bounds.max.x, 2.0F);
  BSE_EXPECT_EQ(bounds.max.y, 3.0F);
}

void NormalizesChildListsAndDropsUnusedResources() {
  auto scene = BuildToolScene();
  bse::Node child;
  child.id = 2;
  child.name = "child";
  child.parent = 1;
  scene.nodes.push_back(child);
  scene.materials.push_back({40, "unused", {1.0F, 1.0F, 1.0F}, 1.0F, 0.0F, 0});
  bse::NormalizeOptions options;
  options.drop_unreferenced_resources = true;
  auto report = bse::NormalizeScene(&scene, options);
  BSE_EXPECT_EQ(report.rebuilt_child_links, 1U);
  BSE_EXPECT_EQ(report.removed_materials, 1U);
  BSE_EXPECT_EQ(scene.nodes.front().children.size(), 1U);
  BSE_EXPECT_EQ(scene.nodes.front().children.front(), 2U);
}

void RoundTripsTextScene() {
  const auto scene = BuildToolScene();
  auto text = bse::WriteTextScene(scene);
  BSE_EXPECT_TRUE(text.ok());
  auto parsed = bse::ParseTextScene(text.value());
  BSE_EXPECT_TRUE(parsed.ok());
  BSE_EXPECT_EQ(parsed.value().name, scene.name);
  BSE_EXPECT_EQ(parsed.value().meshes.size(), 1U);
  BSE_EXPECT_EQ(parsed.value().meshes.front().vertices.size(), 3U);
}

void DiffsSceneChanges() {
  auto left = BuildToolScene();
  auto right = left;
  right.name = "changed";
  right.meshes.front().vertices[1].position.x = 5.0F;
  auto diff = bse::DiffScenes(left, right);
  BSE_EXPECT_TRUE(!diff.empty());
  auto text = bse::FormatDiff(diff);
  BSE_EXPECT_TRUE(text.find("scene.name") != std::string::npos);
  BSE_EXPECT_TRUE(text.find("position") != std::string::npos);
}

void QueriesAcrossSceneSubjects() {
  const auto scene = BuildToolScene();
  auto by_name = bse::QueryByName(scene, "paint");
  BSE_EXPECT_TRUE(!by_name.empty());
  BSE_EXPECT_EQ(by_name.front().subject, bse::QuerySubject::kMaterial);
  auto by_id = bse::QueryById(scene, 30);
  BSE_EXPECT_TRUE(!by_id.empty());
  auto text = bse::FormatQueryResults(by_id);
  BSE_EXPECT_TRUE(text.find("triangle") != std::string::npos);
}

void ComputesWorldTransforms() {
  auto scene = BuildToolScene();
  bse::Node child;
  child.id = 2;
  child.name = "child";
  child.parent = 1;
  child.local.translation = {4.0F, 0.0F, 0.0F};
  scene.nodes.push_back(child);
  bse::NormalizeScene(&scene);
  auto transform = bse::ComputeNodeWorldTransform(scene, 2);
  BSE_EXPECT_TRUE(transform.ok());
  auto point = bse::TransformPoint(transform.value(), {1.0F, 2.0F, 3.0F});
  BSE_EXPECT_EQ(point.x, 5.0F);
  BSE_EXPECT_EQ(point.y, 2.0F);
  BSE_EXPECT_EQ(point.z, 3.0F);
}

void LintsSceneContent() {
  auto scene = BuildToolScene();
  scene.textures.push_back({70, "unused", "unused.ktx2", 8192, 8192, "srgb"});
  auto findings = bse::LintScene(scene);
  BSE_EXPECT_TRUE(!findings.empty());
  auto text = bse::FormatLintFindings(findings);
  BSE_EXPECT_TRUE(text.find("unused-texture") != std::string::npos);
  BSE_EXPECT_TRUE(text.find("large-texture") != std::string::npos);
}

}  // namespace

int main() {
  ComputesStatsAndBounds();
  NormalizesChildListsAndDropsUnusedResources();
  RoundTripsTextScene();
  DiffsSceneChanges();
  QueriesAcrossSceneSubjects();
  ComputesWorldTransforms();
  LintsSceneContent();
  return 0;
}
