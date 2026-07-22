#include "bse/parser.hpp"
#include "bse/serializer.hpp"
#include "bse/scene.hpp"

#include "test_support.hpp"

namespace {

bse::Scene BuildScene() {
  bse::Scene scene = bse::MakeMinimalScene("roundtrip");
  scene.textures.push_back({10, "albedo", "textures/albedo.ktx2", 4, 4, "srgb"});
  scene.materials.push_back({20, "paint", {0.8F, 0.2F, 0.1F}, 0.45F, 0.0F, 10});
  bse::Mesh mesh;
  mesh.id = 30;
  mesh.name = "triangle";
  mesh.material = 20;
  mesh.vertices.push_back({{0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 1.0F}, {0.0F, 0.0F}});
  mesh.vertices.push_back({{1.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 1.0F}, {1.0F, 0.0F}});
  mesh.vertices.push_back({{0.0F, 1.0F, 0.0F}, {0.0F, 0.0F, 1.0F}, {0.0F, 1.0F}});
  mesh.indices = {0, 1, 2};
  scene.meshes.push_back(mesh);
  scene.nodes.front().mesh = 30;
  return scene;
}

void RoundTripsScene() {
  auto bytes = bse::SerializeScene(BuildScene());
  BSE_EXPECT_TRUE(bytes.ok());
  auto parsed = bse::ParseScene(bytes.value());
  BSE_EXPECT_TRUE(parsed.ok());
  BSE_EXPECT_EQ(parsed.value().name, "roundtrip");
  BSE_EXPECT_EQ(parsed.value().nodes.size(), 1U);
  BSE_EXPECT_EQ(parsed.value().meshes.size(), 1U);
  BSE_EXPECT_EQ(parsed.value().materials.size(), 1U);
  BSE_EXPECT_EQ(parsed.value().textures.size(), 1U);
}

void SerializesMetadataDeterministically() {
  auto left = BuildScene();
  left.metadata.clear();
  left.metadata.emplace("zeta", "last");
  left.metadata.emplace("alpha", "first");
  left.metadata.emplace("middle", "center");

  auto right = BuildScene();
  right.metadata.clear();
  right.metadata.emplace("middle", "center");
  right.metadata.emplace("zeta", "last");
  right.metadata.emplace("alpha", "first");

  auto left_bytes = bse::SerializeScene(left);
  auto right_bytes = bse::SerializeScene(right);
  BSE_EXPECT_TRUE(left_bytes.ok());
  BSE_EXPECT_TRUE(right_bytes.ok());
  BSE_EXPECT_EQ(left_bytes.value(), right_bytes.value());
}

void RejectsBadMagic() {
  std::vector<std::uint8_t> bytes{'B', 'A', 'D'};
  auto parsed = bse::ParseScene(bytes);
  BSE_EXPECT_TRUE(!parsed.ok());
}

}  // namespace

int main() {
  RoundTripsScene();
  SerializesMetadataDeterministically();
  RejectsBadMagic();
  return 0;
}
