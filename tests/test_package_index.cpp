#include "bse/package_index.hpp"

#include "test_support.hpp"

namespace {

void WritesAndParsesManifest() {
  bse::PackageIndex index;
  index.name = "demo";
  index.entries.push_back({"scenes/main.bsen", bse::PackageEntryKind::kScene, 0, 128, 10});
  index.entries.push_back({"textures/albedo.ktx2", bse::PackageEntryKind::kTexture, 128, 256, 20});
  const auto text = bse::WritePackageManifest(index);
  auto parsed = bse::ParsePackageManifest(text);
  BSE_EXPECT_TRUE(parsed.ok());
  BSE_EXPECT_EQ(parsed.value().entries.size(), 2U);
  auto scene = bse::FindEntryByPath(parsed.value(), "scenes/main.bsen");
  BSE_EXPECT_TRUE(scene.ok());
  BSE_EXPECT_EQ(scene.value().kind, bse::PackageEntryKind::kScene);
}

void RejectsOverlappingEntries() {
  bse::PackageIndex index;
  index.name = "bad";
  index.entries.push_back({"a.bsen", bse::PackageEntryKind::kScene, 0, 10, 0});
  index.entries.push_back({"b.bsen", bse::PackageEntryKind::kScene, 5, 10, 0});
  auto status = bse::ValidatePackageIndex(index);
  BSE_EXPECT_TRUE(!status.ok());
}

void ComputesStableChecksums() {
  const std::vector<std::uint8_t> bytes{1, 2, 3, 4, 5};
  auto first = bse::ComputeFnv1a64(bytes);
  auto second = bse::ComputeFnv1a64(bytes);
  BSE_EXPECT_EQ(first, second);
  BSE_EXPECT_TRUE(first != 0U);
}

}  // namespace

int main() {
  WritesAndParsesManifest();
  RejectsOverlappingEntries();
  ComputesStableChecksums();
  return 0;
}
