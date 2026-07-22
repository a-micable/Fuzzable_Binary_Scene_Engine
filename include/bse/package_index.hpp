#pragma once

#include "bse/status.hpp"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace bse {

enum class PackageEntryKind {
  kScene,
  kTexture,
  kMaterialLibrary,
  kMetadata,
  kUnknown
};

struct PackageEntry {
  std::string path;
  PackageEntryKind kind = PackageEntryKind::kUnknown;
  std::uint64_t offset = 0;
  std::uint64_t size = 0;
  std::uint64_t checksum = 0;
};

struct PackageIndex {
  std::string name;
  std::uint32_t version = 1;
  std::vector<PackageEntry> entries;
};

PackageEntryKind GuessPackageEntryKind(const std::string& path);
std::string PackageEntryKindName(PackageEntryKind kind);
Result<PackageEntryKind> ParsePackageEntryKind(const std::string& name);
std::uint64_t ComputeFnv1a64(const std::vector<std::uint8_t>& bytes);
Result<PackageIndex> ParsePackageManifest(const std::string& text);
std::string WritePackageManifest(const PackageIndex& index);
std::vector<PackageEntry> FindEntriesByKind(const PackageIndex& index, PackageEntryKind kind);
Result<PackageEntry> FindEntryByPath(const PackageIndex& index, const std::string& path);
Result<void*> ValidatePackageIndex(const PackageIndex& index);

}  // namespace bse
