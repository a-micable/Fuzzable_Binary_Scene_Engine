#include "bse/asset_catalog.hpp"

#include <algorithm>
#include <sstream>
#include <unordered_map>

namespace bse {

namespace {

std::string DirectoryName(const std::string& uri) {
  const auto slash = uri.find_last_of("/\\");
  if (slash == std::string::npos) {
    return ".";
  }
  return uri.substr(0, slash);
}

bool IsExternalUri(const std::string& uri) {
  return uri.find("://") != std::string::npos || (!uri.empty() && uri.front() == '/');
}

AssetRecord* FindRecord(std::vector<AssetRecord>* records, AssetKind kind, ObjectId id) {
  for (auto& record : *records) {
    if (record.kind == kind && record.id == id) {
      return &record;
    }
  }
  return nullptr;
}

void AddReference(std::vector<AssetRecord>* records, AssetKind kind, ObjectId id, ObjectId owner) {
  if (auto* record = FindRecord(records, kind, id)) {
    record->referenced_by.push_back(owner);
  }
}

}  // namespace

std::string AssetKindName(AssetKind kind) {
  switch (kind) {
    case AssetKind::kTexture:
      return "texture";
    case AssetKind::kMaterial:
      return "material";
    case AssetKind::kMesh:
      return "mesh";
    case AssetKind::kCamera:
      return "camera";
    case AssetKind::kLight:
      return "light";
    case AssetKind::kAnimation:
      return "animation";
    case AssetKind::kSkeleton:
      return "skeleton";
  }
  return "unknown";
}

AssetCatalog BuildAssetCatalog(const Scene& scene) {
  AssetCatalog catalog;
  for (const auto& texture : scene.textures) {
    catalog.records.push_back({AssetKind::kTexture, texture.id, texture.name, texture.uri, {},
                               IsExternalUri(texture.uri)});
  }
  for (const auto& material : scene.materials) {
    catalog.records.push_back({AssetKind::kMaterial, material.id, material.name, {}, {}, false});
    if (material.base_color_texture != kInvalidObjectId) {
      AddReference(&catalog.records, AssetKind::kTexture, material.base_color_texture, material.id);
    }
  }
  for (const auto& mesh : scene.meshes) {
    catalog.records.push_back({AssetKind::kMesh, mesh.id, mesh.name, {}, {}, false});
    if (mesh.material != kInvalidObjectId) {
      AddReference(&catalog.records, AssetKind::kMaterial, mesh.material, mesh.id);
    }
  }
  for (const auto& camera : scene.cameras) {
    catalog.records.push_back({AssetKind::kCamera, camera.id, camera.name, {}, {}, false});
  }
  for (const auto& light : scene.lights) {
    catalog.records.push_back({AssetKind::kLight, light.id, light.name, {}, {}, false});
  }
  for (const auto& animation : scene.animations) {
    catalog.records.push_back({AssetKind::kAnimation, animation.id, animation.name, {}, {}, false});
  }
  for (const auto& skeleton : scene.skeletons) {
    catalog.records.push_back({AssetKind::kSkeleton, skeleton.id, skeleton.name, {}, {}, false});
  }
  for (const auto& node : scene.nodes) {
    if (node.mesh != kInvalidObjectId) {
      AddReference(&catalog.records, AssetKind::kMesh, node.mesh, node.id);
    }
    if (node.camera != kInvalidObjectId) {
      AddReference(&catalog.records, AssetKind::kCamera, node.camera, node.id);
    }
    if (node.light != kInvalidObjectId) {
      AddReference(&catalog.records, AssetKind::kLight, node.light, node.id);
    }
  }
  for (const auto& animation : scene.animations) {
    for (const auto& channel : animation.channels) {
      if (channel.target_node != kInvalidObjectId) {
        AddReference(&catalog.records, AssetKind::kAnimation, animation.id, channel.target_node);
      }
    }
  }
  std::stable_sort(catalog.records.begin(), catalog.records.end(), [](const auto& lhs, const auto& rhs) {
    if (lhs.kind != rhs.kind) {
      return static_cast<int>(lhs.kind) < static_cast<int>(rhs.kind);
    }
    return lhs.id < rhs.id;
  });
  return catalog;
}

std::vector<AssetRecord> FindExternalAssets(const AssetCatalog& catalog) {
  std::vector<AssetRecord> out;
  for (const auto& record : catalog.records) {
    if (record.external) {
      out.push_back(record);
    }
  }
  return out;
}

std::vector<AssetRecord> FindUnreferencedAssets(const AssetCatalog& catalog) {
  std::vector<AssetRecord> out;
  for (const auto& record : catalog.records) {
    if (record.referenced_by.empty() && record.kind != AssetKind::kAnimation &&
        record.kind != AssetKind::kSkeleton) {
      out.push_back(record);
    }
  }
  return out;
}

std::vector<AssetRecord> FindAssetsByUriPrefix(const AssetCatalog& catalog, const std::string& prefix) {
  std::vector<AssetRecord> out;
  for (const auto& record : catalog.records) {
    if (record.uri.rfind(prefix, 0) == 0) {
      out.push_back(record);
    }
  }
  return out;
}

std::unordered_map<std::string, std::vector<AssetRecord>> GroupAssetsByDirectory(
    const AssetCatalog& catalog) {
  std::unordered_map<std::string, std::vector<AssetRecord>> groups;
  for (const auto& record : catalog.records) {
    if (!record.uri.empty()) {
      groups[DirectoryName(record.uri)].push_back(record);
    }
  }
  return groups;
}

std::string FormatAssetCatalog(const AssetCatalog& catalog) {
  std::ostringstream out;
  for (const auto& record : catalog.records) {
    out << AssetKindName(record.kind) << " " << record.id << " " << record.name;
    if (!record.uri.empty()) {
      out << " uri=" << record.uri;
    }
    out << " refs=" << record.referenced_by.size();
    if (record.external) {
      out << " external";
    }
    out << "\n";
  }
  return out.str();
}

Result<void*> ValidateAssetCatalog(const AssetCatalog& catalog) {
  std::unordered_map<std::string, ObjectId> uri_owners;
  for (const auto& record : catalog.records) {
    if (record.id == kInvalidObjectId) {
      return Status::Error(ErrorCode::kValidationFailed, "asset catalog contains id 0");
    }
    if (!record.uri.empty()) {
      auto inserted = uri_owners.emplace(record.uri, record.id);
      if (!inserted.second && inserted.first->second != record.id) {
        return Status::Error(ErrorCode::kValidationFailed, "asset URI is used by multiple records");
      }
    }
  }
  return static_cast<void*>(nullptr);
}

}  // namespace bse
