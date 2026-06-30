#pragma once

#include "bse/scene.hpp"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace bse {

struct SignatureEntry {
  std::string path;
  std::string value;
};

struct SceneSignature {
  std::uint64_t hash = 0;
  std::vector<SignatureEntry> entries;
};

enum class SignatureDeltaKind {
  kAdded,
  kRemoved,
  kChanged
};

struct SignatureDelta {
  SignatureDeltaKind kind = SignatureDeltaKind::kChanged;
  std::string path;
  std::string left_value;
  std::string right_value;
};

struct SignatureDiff {
  std::string left_digest;
  std::string right_digest;
  std::vector<SignatureDelta> deltas;
};

struct SignatureDiffSummary {
  std::size_t added = 0;
  std::size_t removed = 0;
  std::size_t changed = 0;
  std::size_t node_changes = 0;
  std::size_t mesh_changes = 0;
  std::size_t material_changes = 0;
  std::size_t texture_changes = 0;
  std::size_t animation_changes = 0;
  std::size_t metadata_changes = 0;
};

struct SignatureIndexEntry {
  std::string name;
  std::string digest;
  std::size_t entry_count = 0;
};

struct SignatureIndexStats {
  std::size_t entries = 0;
  std::size_t unique_digests = 0;
  std::size_t duplicate_digests = 0;
  std::size_t duplicate_entries = 0;
  std::size_t largest_group = 0;
};

class SceneSignatureIndex {
 public:
  bool Add(std::string name, const SceneSignature& signature);
  bool Remove(const std::string& name);
  void Clear();
  bool ContainsDigest(const std::string& digest) const;
  bool ContainsName(const std::string& name) const;
  std::string DigestForName(const std::string& name) const;
  std::vector<std::string> NamesForDigest(const std::string& digest) const;
  std::vector<SignatureIndexEntry> UniqueEntries() const;
  std::vector<SignatureIndexEntry> Entries() const;
  std::vector<std::vector<std::string>> DuplicateGroups() const;
  std::vector<std::string> DuplicateDigests() const;
  std::string FirstDuplicateDigest() const;
  SignatureIndexStats Stats() const;
  bool HasDuplicates() const;
  bool IsUnique() const;
  std::size_t DuplicateEntryCount() const;
  std::size_t DigestCount(const std::string& digest) const;
  bool empty() const { return entries_.empty(); }
  std::size_t size() const { return entries_.size(); }

 private:
  std::vector<SignatureIndexEntry> entries_;
};

struct SignatureOptions {
  bool include_names = true;
  bool include_metadata = true;
  bool include_geometry = true;
  bool include_animation = true;
  bool stable_float_format = true;
};

SceneSignature ComputeSceneSignature(const Scene& scene,
                                     const SignatureOptions& options = SignatureOptions{});
std::string FormatSceneSignature(const SceneSignature& signature);
std::string SceneSignatureDigest(const SceneSignature& signature);
bool SceneSignaturesEquivalent(const SceneSignature& left, const SceneSignature& right);
SignatureDiff DiffSceneSignatures(const SceneSignature& left, const SceneSignature& right);
SignatureDiffSummary SummarizeSignatureDiff(const SignatureDiff& diff);
std::string SignatureDeltaKindName(SignatureDeltaKind kind);
std::string FormatSignatureDiffSummary(const SignatureDiffSummary& summary);
std::string FormatSignatureDiff(const SignatureDiff& diff);
std::string FormatSignatureIndexStats(const SignatureIndexStats& stats);
std::string FormatSignatureIndex(const SceneSignatureIndex& index);

}  // namespace bse
