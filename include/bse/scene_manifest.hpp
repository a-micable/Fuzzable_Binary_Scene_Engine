#pragma once

#include "bse/scene.hpp"

#include <cstddef>
#include <string>
#include <vector>

namespace bse {

enum class ManifestResourceKind {
  kNode,
  kMesh,
  kMaterial,
  kTexture,
  kCamera,
  kLight,
  kSkeleton,
  kAnimation
};

struct ManifestResource {
  ManifestResourceKind kind = ManifestResourceKind::kNode;
  ObjectId id = kInvalidObjectId;
  std::string name;
  std::size_t payload_count = 0;
};

struct ManifestReference {
  ManifestResourceKind from_kind = ManifestResourceKind::kNode;
  ObjectId from = kInvalidObjectId;
  ManifestResourceKind to_kind = ManifestResourceKind::kNode;
  ObjectId to = kInvalidObjectId;
  std::string label;
  bool resolved = false;
};

struct SceneManifest {
  std::string scene_name;
  std::vector<ManifestResource> resources;
  std::vector<ManifestReference> references;
};

struct ManifestSummary {
  std::size_t resources = 0;
  std::size_t references = 0;
  std::size_t unresolved_references = 0;
  std::size_t nodes = 0;
  std::size_t meshes = 0;
  std::size_t materials = 0;
  std::size_t textures = 0;
  std::size_t animations = 0;
};

struct ManifestResourceUsage {
  ManifestResourceKind kind = ManifestResourceKind::kNode;
  ObjectId id = kInvalidObjectId;
  std::string name;
  std::size_t incoming = 0;
  std::size_t outgoing = 0;
  bool unresolved = false;
};

SceneManifest BuildSceneManifest(const Scene& scene);
std::vector<ManifestResource> FindManifestResources(const SceneManifest& manifest,
                                                    ManifestResourceKind kind);
std::vector<ManifestReference> FindUnresolvedReferences(const SceneManifest& manifest);
std::vector<ManifestReference> IncomingReferences(const SceneManifest& manifest,
                                                  ManifestResourceKind kind, ObjectId id);
std::vector<ManifestReference> OutgoingReferences(const SceneManifest& manifest,
                                                  ManifestResourceKind kind, ObjectId id);
std::vector<ManifestResource> FindUnreferencedResources(const SceneManifest& manifest,
                                                        ManifestResourceKind kind);
std::vector<ManifestResourceUsage> BuildManifestUsage(const SceneManifest& manifest);
std::vector<ManifestResourceUsage> FindUnusedManifestResources(const SceneManifest& manifest);
bool ManifestContainsResource(const SceneManifest& manifest, ManifestResourceKind kind,
                              ObjectId id);
bool ManifestHasResources(const SceneManifest& manifest);
bool ManifestHasReferences(const SceneManifest& manifest);
bool ManifestHasUnresolvedReferences(const SceneManifest& manifest);
std::size_t CountResolvedReferences(const SceneManifest& manifest);
std::size_t CountUnresolvedReferences(const SceneManifest& manifest);
std::size_t CountManifestResources(const SceneManifest& manifest, ManifestResourceKind kind);
ManifestSummary SummarizeManifest(const SceneManifest& manifest);
std::string ManifestResourceKindName(ManifestResourceKind kind);
std::string FormatManifestSummary(const ManifestSummary& summary);
std::string FormatManifestUsage(const std::vector<ManifestResourceUsage>& usage);
std::string FormatSceneManifest(const SceneManifest& manifest);
std::string WriteManifestCsv(const SceneManifest& manifest);
std::string WriteManifestJson(const SceneManifest& manifest);
std::string WriteManifestMarkdown(const SceneManifest& manifest);
std::string WriteManifestDot(const SceneManifest& manifest);
std::vector<std::string> ValidateManifestConsistency(const SceneManifest& manifest);

}  // namespace bse
