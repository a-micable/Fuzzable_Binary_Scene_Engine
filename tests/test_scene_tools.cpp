#include "bse/animation_tools.hpp"
#include "bse/asset_catalog.hpp"
#include "bse/binary_inspector.hpp"
#include "bse/corpus_tools.hpp"
#include "bse/mesh_tools.hpp"
#include "bse/obj_io.hpp"
#include "bse/scene_builder.hpp"
#include "bse/scene_collection.hpp"
#include "bse/scene_diff.hpp"
#include "bse/scene_graph.hpp"
#include "bse/scene_lint.hpp"
#include "bse/scene_manifest.hpp"
#include "bse/scene_metrics.hpp"
#include "bse/scene_mutator.hpp"
#include "bse/scene_ops.hpp"
#include "bse/scene_optimizer.hpp"
#include "bse/scene_query.hpp"
#include "bse/scene_report.hpp"
#include "bse/scene_signature.hpp"
#include "bse/scene_stats.hpp"
#include "bse/serializer.hpp"
#include "bse/text_scene.hpp"
#include "bse/transform.hpp"
#include "bse/validation_profile.hpp"
#include "bse/validator.hpp"

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

void BuildsProceduralMeshesAndScenes() {
  bse::MeshBuildOptions options;
  options.name = "box";
  auto box = bse::MakeBoxMesh(2.0F, 2.0F, 2.0F, options);
  BSE_EXPECT_EQ(box.indices.size(), 36U);
  auto repair = bse::RepairMesh(&box);
  BSE_EXPECT_EQ(repair.removed_out_of_range_indices, 0U);
  auto scene = bse::MakePrimitiveShowcaseScene();
  BSE_EXPECT_TRUE(!scene.meshes.empty());
  BSE_EXPECT_TRUE(!scene.animations.empty());
  auto status = bse::ValidateScene(scene);
  BSE_EXPECT_TRUE(status.ok());
}

void SamplesAnimationsAndAppliesPose() {
  auto scene = bse::MakePrimitiveShowcaseScene();
  const auto& animation = scene.animations.front();
  auto samples = bse::SampleAnimation(animation, 0.5F);
  BSE_EXPECT_TRUE(samples.ok());
  BSE_EXPECT_TRUE(!samples.value().empty());
  auto baked = bse::BakeAnimation(animation);
  BSE_EXPECT_TRUE(baked.ok());
  BSE_EXPECT_TRUE(!baked.value().frames.empty());
  auto posed = bse::ApplyAnimationPose(scene, animation, animation.duration_seconds);
  BSE_EXPECT_TRUE(posed.ok());
}

void RoundTripsObjMesh() {
  bse::MeshBuildOptions options;
  options.name = "plane";
  auto mesh = bse::MakePlaneMesh(2.0F, 2.0F, 1, 1, options);
  auto text = bse::WriteObjMesh(mesh);
  BSE_EXPECT_TRUE(text.ok());
  auto parsed = bse::ReadObjMesh(text.value());
  BSE_EXPECT_TRUE(parsed.ok());
  BSE_EXPECT_EQ(parsed.value().indices.size(), mesh.indices.size());
}

void BuildsAssetCatalogAndReports() {
  auto scene = BuildToolScene();
  auto catalog = bse::BuildAssetCatalog(scene);
  auto valid = bse::ValidateAssetCatalog(catalog);
  BSE_EXPECT_TRUE(valid.ok());
  auto formatted = bse::FormatAssetCatalog(catalog);
  BSE_EXPECT_TRUE(formatted.find("texture") != std::string::npos);
  auto markdown = bse::WriteMarkdownReport(scene);
  auto json = bse::WriteJsonReport(scene);
  BSE_EXPECT_TRUE(markdown.find("Scene Report") != std::string::npos);
  BSE_EXPECT_TRUE(json.find("\"stats\"") != std::string::npos);
}

void ManagesSceneCollectionsAndCorpusCases() {
  bse::SceneCollection collection;
  BSE_EXPECT_TRUE(collection.Add("grid", bse::MakeGridScene(2, 2)));
  BSE_EXPECT_TRUE(collection.Add("showcase", bse::MakePrimitiveShowcaseScene()));
  auto stats = collection.Stats();
  BSE_EXPECT_EQ(stats.scene_count, 2U);
  BSE_EXPECT_TRUE(stats.total_meshes > 0U);
  auto animated = collection.FindScenesWithAnimation();
  BSE_EXPECT_TRUE(!animated.empty());
  auto diff = collection.Diff("grid", "showcase");
  BSE_EXPECT_TRUE(diff.ok());
  auto merged = collection.MergeAll("merged");
  BSE_EXPECT_TRUE(merged.ok());

  auto cases = bse::GenerateSceneCorpus();
  BSE_EXPECT_TRUE(cases.size() >= 3U);
  auto bytes = bse::SerializeCorpus(cases);
  BSE_EXPECT_TRUE(bytes.ok());
  BSE_EXPECT_EQ(bytes.value().size(), cases.size());
  auto manifest = bse::CorpusManifest(cases);
  BSE_EXPECT_TRUE(manifest.find("grid") != std::string::npos);
}

void InspectsBinarySceneFields() {
  auto bytes = bse::SerializeScene(BuildToolScene());
  BSE_EXPECT_TRUE(bytes.ok());
  auto inspection = bse::InspectBinaryScene(bytes.value());
  BSE_EXPECT_TRUE(inspection.ok());
  BSE_EXPECT_TRUE(inspection.value().recognized);
  auto text = bse::FormatBinaryInspection(inspection.value());
  BSE_EXPECT_TRUE(text.find("scene.name") != std::string::npos);
  BSE_EXPECT_TRUE(text.find("meshes.count") != std::string::npos);
}

void MutatesScenesForCorpusDepth() {
  auto scene = bse::MakePrimitiveShowcaseScene();
  bse::MutationRecipe recipe;
  recipe.name = "test-mutation";
  recipe.steps = {bse::MutationKind::kDuplicateMesh, bse::MutationKind::kDuplicateSubtree,
                  bse::MutationKind::kPerturbGeometry, bse::MutationKind::kExpandAnimationKeys};
  auto mutated = bse::MutateScene(scene, recipe);
  BSE_EXPECT_TRUE(mutated.ok());
  BSE_EXPECT_TRUE(mutated.value().report.meshes_added > 0U);
  BSE_EXPECT_TRUE(mutated.value().scene.meshes.size() >= scene.meshes.size());
  BSE_EXPECT_TRUE(!bse::FormatMutationReport(mutated.value().report).empty());
  auto suite = bse::GenerateMutationSuite(scene);
  BSE_EXPECT_TRUE(suite.size() >= 4U);
}

void ChecksProfilesAndComplexity() {
  auto scene = bse::MakePrimitiveShowcaseScene();
  auto profile = bse::StrictAssetProfile();
  auto report = bse::CheckValidationProfile(scene, profile);
  BSE_EXPECT_TRUE(!report.profile_name.empty());
  BSE_EXPECT_TRUE(!bse::FormatProfileReport(report).empty());
  auto metrics = bse::ComputeSceneComplexity(scene);
  BSE_EXPECT_TRUE(metrics.complexity_score > 0.0F);
  BSE_EXPECT_TRUE(!metrics.meshes.empty());
  auto formatted = bse::FormatSceneComplexity(metrics);
  BSE_EXPECT_TRUE(formatted.find("meshes") != std::string::npos);
}

void OptimizesScenePayloads() {
  auto scene = BuildToolScene();
  scene.textures.push_back({11, "albedo-copy", "textures/albedo.ktx2", 128, 128, "srgb"});
  scene.materials.push_back({21, "paint-copy", {0.4F, 0.5F, 0.6F}, 0.8F, 0.1F, 10});
  scene.animations.push_back({90, "empty", 0.0F, {}});
  bse::Mesh degenerate = scene.meshes.front();
  degenerate.id = 31;
  degenerate.name = "degenerate";
  degenerate.indices = {0, 0, 1};
  scene.meshes.push_back(degenerate);
  scene.nodes.front().mesh = 31;
  auto optimized = bse::OptimizeScene(scene);
  BSE_EXPECT_TRUE(optimized.ok());
  BSE_EXPECT_TRUE(optimized.value().report.removed_triangles > 0U);
  BSE_EXPECT_TRUE(optimized.value().report.removed_animations > 0U);
  auto text = bse::FormatOptimizationReport(optimized.value().report);
  BSE_EXPECT_TRUE(text.find("remove-degenerate-triangles") != std::string::npos);
}

void ComputesDeterministicSignatures() {
  auto scene = BuildToolScene();
  auto left = bse::ComputeSceneSignature(scene);
  auto right = bse::ComputeSceneSignature(scene);
  BSE_EXPECT_TRUE(bse::SceneSignaturesEquivalent(left, right));
  BSE_EXPECT_TRUE(!bse::SceneSignatureDigest(left).empty());
  auto text = bse::FormatSceneSignature(left);
  BSE_EXPECT_TRUE(text.find("scene.name") != std::string::npos);
  scene.name = "changed";
  auto changed = bse::ComputeSceneSignature(scene);
  BSE_EXPECT_TRUE(!bse::SceneSignaturesEquivalent(left, changed));
  auto diff = bse::DiffSceneSignatures(left, changed);
  BSE_EXPECT_TRUE(!diff.deltas.empty());
  auto summary = bse::SummarizeSignatureDiff(diff);
  BSE_EXPECT_TRUE(summary.changed > 0U);
  BSE_EXPECT_TRUE(bse::FormatSignatureDiff(diff).find("changed") != std::string::npos);
  bse::SceneSignatureIndex index;
  BSE_EXPECT_TRUE(index.Add("left", left));
  BSE_EXPECT_TRUE(index.Add("left-copy", left));
  BSE_EXPECT_TRUE(index.Add("changed", changed));
  BSE_EXPECT_EQ(index.size(), 3U);
  BSE_EXPECT_TRUE(index.ContainsName("left"));
  BSE_EXPECT_TRUE(index.ContainsDigest(bse::SceneSignatureDigest(left)));
  BSE_EXPECT_EQ(index.DigestForName("left"), bse::SceneSignatureDigest(left));
  BSE_EXPECT_EQ(index.DigestCount(bse::SceneSignatureDigest(left)), 2U);
  BSE_EXPECT_EQ(index.UniqueEntries().size(), 2U);
  BSE_EXPECT_TRUE(!index.DuplicateDigests().empty());
  BSE_EXPECT_TRUE(!index.FirstDuplicateDigest().empty());
  BSE_EXPECT_TRUE(!index.DuplicateGroups().empty());
  auto index_stats = index.Stats();
  BSE_EXPECT_EQ(index_stats.entries, 3U);
  BSE_EXPECT_TRUE(index_stats.duplicate_entries > 0U);
  BSE_EXPECT_TRUE(index.HasDuplicates());
  BSE_EXPECT_TRUE(!index.IsUnique());
  BSE_EXPECT_EQ(index.DuplicateEntryCount(), 2U);
  BSE_EXPECT_TRUE(bse::FormatSignatureIndex(index).find("duplicate") != std::string::npos);
  BSE_EXPECT_TRUE(index.Remove("changed"));
  BSE_EXPECT_EQ(index.size(), 2U);
}

void AnalyzesGraphsAndManifests() {
  auto scene = BuildToolScene();
  bse::Node child;
  child.id = 2;
  child.name = "child";
  child.parent = 1;
  scene.nodes.push_back(child);
  bse::NormalizeScene(&scene);
  auto graph = bse::BuildSceneGraph(scene);
  BSE_EXPECT_TRUE(bse::GraphContainsNode(graph, 1));
  BSE_EXPECT_TRUE(bse::GraphHasRoots(graph));
  BSE_EXPECT_TRUE(!bse::GraphHasCycles(graph));
  BSE_EXPECT_TRUE(!bse::GraphHasUnreachableNodes(graph));
  BSE_EXPECT_TRUE(bse::GraphIsForest(graph));
  BSE_EXPECT_EQ(bse::GraphEdgeCount(graph), 1U);
  BSE_EXPECT_EQ(bse::GraphRootCount(graph), 1U);
  BSE_EXPECT_EQ(bse::DescendantsOf(graph, 1).size(), 1U);
  BSE_EXPECT_EQ(bse::AncestorsOf(graph, 2).size(), 1U);
  BSE_EXPECT_EQ(bse::LeafNodes(graph).size(), 1U);
  BSE_EXPECT_EQ(bse::NodesAtDepth(graph, 1).size(), 1U);
  BSE_EXPECT_EQ(bse::ReachableFrom(graph, 1).size(), 2U);
  BSE_EXPECT_EQ(bse::ConnectedComponents(graph).size(), 1U);
  BSE_EXPECT_TRUE(!bse::GraphDepthHistogram(graph).empty());
  BSE_EXPECT_EQ(bse::SubtreeSize(graph, 1), 2U);
  BSE_EXPECT_EQ(bse::LowestCommonAncestor(graph, 1, 2), 1U);
  BSE_EXPECT_TRUE(bse::TopologicalNodeOrder(graph).ok());
  BSE_EXPECT_EQ(bse::SummarizeSceneGraph(graph).node_count, 2U);
  BSE_EXPECT_TRUE(bse::FormatSceneGraph(graph).find("roots") != std::string::npos);
  BSE_EXPECT_TRUE(bse::WriteGraphDot(graph).find("digraph") != std::string::npos);
  BSE_EXPECT_TRUE(bse::WriteGraphAdjacencyList(graph).find("1:") != std::string::npos);
  BSE_EXPECT_TRUE(bse::WriteGraphCsv(graph).find("reachable") != std::string::npos);
  BSE_EXPECT_TRUE(bse::WriteGraphJson(graph).find("\"nodes\"") != std::string::npos);
  BSE_EXPECT_TRUE(!bse::FormatGraphTraversal(bse::TraverseDepthFirst(graph)).empty());
  BSE_EXPECT_TRUE(bse::ValidateGraphConsistency(graph).empty());

  auto manifest = bse::BuildSceneManifest(scene);
  BSE_EXPECT_TRUE(bse::ManifestHasResources(manifest));
  BSE_EXPECT_TRUE(bse::ManifestHasReferences(manifest));
  BSE_EXPECT_TRUE(!manifest.resources.empty());
  BSE_EXPECT_TRUE(!manifest.references.empty());
  BSE_EXPECT_TRUE(bse::FindUnresolvedReferences(manifest).empty());
  auto meshes = bse::FindManifestResources(manifest, bse::ManifestResourceKind::kMesh);
  BSE_EXPECT_EQ(meshes.size(), 1U);
  BSE_EXPECT_TRUE(bse::ManifestContainsResource(manifest, bse::ManifestResourceKind::kMesh, 30));
  BSE_EXPECT_TRUE(!bse::ManifestHasUnresolvedReferences(manifest));
  BSE_EXPECT_TRUE(bse::CountResolvedReferences(manifest) > 0U);
  BSE_EXPECT_EQ(bse::CountUnresolvedReferences(manifest), 0U);
  BSE_EXPECT_TRUE(!bse::IncomingReferences(manifest, bse::ManifestResourceKind::kMesh, 30).empty());
  BSE_EXPECT_TRUE(!bse::OutgoingReferences(manifest, bse::ManifestResourceKind::kNode, 1).empty());
  BSE_EXPECT_TRUE(!bse::BuildManifestUsage(manifest).empty());
  BSE_EXPECT_EQ(bse::SummarizeManifest(manifest).meshes, 1U);
  BSE_EXPECT_TRUE(bse::FormatSceneManifest(manifest).find("references") != std::string::npos);
  BSE_EXPECT_TRUE(bse::WriteManifestCsv(manifest).find("resource") != std::string::npos);
  BSE_EXPECT_TRUE(bse::WriteManifestJson(manifest).find("\"resources\"") != std::string::npos);
  BSE_EXPECT_TRUE(bse::WriteManifestMarkdown(manifest).find("Resources") != std::string::npos);
  BSE_EXPECT_TRUE(bse::WriteManifestDot(manifest).find("digraph") != std::string::npos);
  BSE_EXPECT_TRUE(bse::ValidateManifestConsistency(manifest).empty());

  auto cyclic = scene;
  cyclic.nodes.front().children = {2};
  cyclic.nodes.back().children = {1};
  auto cyclic_graph = bse::BuildSceneGraph(cyclic);
  BSE_EXPECT_TRUE(bse::GraphHasCycles(cyclic_graph));
  BSE_EXPECT_EQ(bse::TraverseDepthFirst(cyclic_graph).size(), 2U);
  BSE_EXPECT_EQ(bse::TraverseBreadthFirst(cyclic_graph).size(), 2U);
  BSE_EXPECT_EQ(bse::DescendantsOf(cyclic_graph, 1).size(), 1U);
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
  BuildsProceduralMeshesAndScenes();
  SamplesAnimationsAndAppliesPose();
  RoundTripsObjMesh();
  BuildsAssetCatalogAndReports();
  ManagesSceneCollectionsAndCorpusCases();
  InspectsBinarySceneFields();
  MutatesScenesForCorpusDepth();
  ChecksProfilesAndComplexity();
  OptimizesScenePayloads();
  ComputesDeterministicSignatures();
  AnalyzesGraphsAndManifests();
  return 0;
}
