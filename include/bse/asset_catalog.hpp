#pragma once

#include "bse/scene.hpp"
#include "bse/status.hpp"

#include <string>
#include <unordered_map>
#include <vector>

namespace bse {

enum class AssetKind {
  kTexture,
  kMaterial,
  kMesh,
  kCamera,
  kLight,
  kAnimation,
  kSkeleton
};

struct AssetRecord {
  AssetKind kind = AssetKind::kTexture;
  ObjectId id = kInvalidObjectId;
  std::string name;
  std::string uri;
  std::vector<ObjectId> referenced_by;
  bool external = false;
};

struct AssetCatalog {
  std::vector<AssetRecord> records;
};

std::string AssetKindName(AssetKind kind);
AssetCatalog BuildAssetCatalog(const Scene& scene);
std::vector<AssetRecord> FindExternalAssets(const AssetCatalog& catalog);
std::vector<AssetRecord> FindUnreferencedAssets(const AssetCatalog& catalog);
std::vector<AssetRecord> FindAssetsByUriPrefix(const AssetCatalog& catalog, const std::string& prefix);
std::unordered_map<std::string, std::vector<AssetRecord>> GroupAssetsByDirectory(
    const AssetCatalog& catalog);
std::string FormatAssetCatalog(const AssetCatalog& catalog);
Result<void*> ValidateAssetCatalog(const AssetCatalog& catalog);

}  // namespace bse
