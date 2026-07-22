#include "bse/scene_signature.hpp"

#include <algorithm>
#include <iomanip>
#include <map>
#include <sstream>
#include <unordered_set>

namespace bse {

namespace {

constexpr std::uint64_t kFnvOffset = 1469598103934665603ULL;
constexpr std::uint64_t kFnvPrime = 1099511628211ULL;

std::uint64_t HashBytes(std::uint64_t hash, const std::string& value) {
  for (char raw_byte : value) {
    const auto byte = static_cast<unsigned char>(raw_byte);
    hash ^= static_cast<std::uint64_t>(byte);
    hash *= kFnvPrime;
  }
  return hash;
}

std::string FloatText(float value, bool stable) {
  std::ostringstream out;
  if (stable) {
    out << std::fixed << std::setprecision(6);
  }
  out << value;
  return out.str();
}

std::string Vec2Text(Vec2 value, bool stable) {
  return FloatText(value.x, stable) + "," + FloatText(value.y, stable);
}

std::string Vec3Text(Vec3 value, bool stable) {
  return FloatText(value.x, stable) + "," + FloatText(value.y, stable) + "," +
         FloatText(value.z, stable);
}

std::string QuatText(Quat value, bool stable) {
  return FloatText(value.x, stable) + "," + FloatText(value.y, stable) + "," +
         FloatText(value.z, stable) + "," + FloatText(value.w, stable);
}

std::string TransformText(const Transform& transform, bool stable) {
  return "t(" + Vec3Text(transform.translation, stable) + ") r(" +
         QuatText(transform.rotation, stable) + ") s(" + Vec3Text(transform.scale, stable) + ")";
}

void Add(SceneSignature* signature, std::string path, std::string value) {
  signature->entries.push_back({std::move(path), std::move(value)});
}

void AddCount(SceneSignature* signature, const std::string& path, std::size_t value) {
  Add(signature, path, std::to_string(value));
}

std::string Id(ObjectId id) {
  return std::to_string(id);
}

void AddMetadata(SceneSignature* signature, const Scene& scene) {
  std::vector<std::pair<std::string, std::string>> entries(scene.metadata.begin(), scene.metadata.end());
  std::sort(entries.begin(), entries.end());
  AddCount(signature, "metadata.count", entries.size());
  for (const auto& entry : entries) {
    Add(signature, "metadata." + entry.first, entry.second);
  }
}

void AddNodes(SceneSignature* signature, const Scene& scene, const SignatureOptions& options) {
  AddCount(signature, "nodes.count", scene.nodes.size());
  std::vector<Node> nodes = scene.nodes;
  std::sort(nodes.begin(), nodes.end(), [](const Node& left, const Node& right) {
    return left.id < right.id;
  });
  for (const auto& node : nodes) {
    const std::string prefix = "node." + Id(node.id);
    if (options.include_names) Add(signature, prefix + ".name", node.name);
    Add(signature, prefix + ".parent", Id(node.parent));
    Add(signature, prefix + ".mesh", Id(node.mesh));
    Add(signature, prefix + ".camera", Id(node.camera));
    Add(signature, prefix + ".light", Id(node.light));
    Add(signature, prefix + ".local", TransformText(node.local, options.stable_float_format));
    std::vector<ObjectId> children = node.children;
    std::sort(children.begin(), children.end());
    AddCount(signature, prefix + ".children.count", children.size());
    for (std::size_t i = 0; i < children.size(); ++i) {
      Add(signature, prefix + ".children." + std::to_string(i), Id(children[i]));
    }
  }
}

void AddTextures(SceneSignature* signature, const Scene& scene, const SignatureOptions& options) {
  AddCount(signature, "textures.count", scene.textures.size());
  std::vector<Texture> textures = scene.textures;
  std::sort(textures.begin(), textures.end(), [](const Texture& left, const Texture& right) {
    return left.id < right.id;
  });
  for (const auto& texture : textures) {
    const std::string prefix = "texture." + Id(texture.id);
    if (options.include_names) Add(signature, prefix + ".name", texture.name);
    Add(signature, prefix + ".uri", texture.uri);
    Add(signature, prefix + ".width", std::to_string(texture.width));
    Add(signature, prefix + ".height", std::to_string(texture.height));
    Add(signature, prefix + ".color_space", texture.color_space);
  }
}

void AddMaterials(SceneSignature* signature, const Scene& scene, const SignatureOptions& options) {
  AddCount(signature, "materials.count", scene.materials.size());
  std::vector<Material> materials = scene.materials;
  std::sort(materials.begin(), materials.end(), [](const Material& left, const Material& right) {
    return left.id < right.id;
  });
  for (const auto& material : materials) {
    const std::string prefix = "material." + Id(material.id);
    if (options.include_names) Add(signature, prefix + ".name", material.name);
    Add(signature, prefix + ".base_color", Vec3Text(material.base_color, options.stable_float_format));
    Add(signature, prefix + ".roughness", FloatText(material.roughness, options.stable_float_format));
    Add(signature, prefix + ".metallic", FloatText(material.metallic, options.stable_float_format));
    Add(signature, prefix + ".base_color_texture", Id(material.base_color_texture));
  }
}

void AddMeshes(SceneSignature* signature, const Scene& scene, const SignatureOptions& options) {
  AddCount(signature, "meshes.count", scene.meshes.size());
  std::vector<Mesh> meshes = scene.meshes;
  std::sort(meshes.begin(), meshes.end(), [](const Mesh& left, const Mesh& right) {
    return left.id < right.id;
  });
  for (const auto& mesh : meshes) {
    const std::string prefix = "mesh." + Id(mesh.id);
    if (options.include_names) Add(signature, prefix + ".name", mesh.name);
    Add(signature, prefix + ".material", Id(mesh.material));
    AddCount(signature, prefix + ".vertices.count", mesh.vertices.size());
    AddCount(signature, prefix + ".indices.count", mesh.indices.size());
    if (!options.include_geometry) continue;
    for (std::size_t i = 0; i < mesh.vertices.size(); ++i) {
      const auto& vertex = mesh.vertices[i];
      const std::string vertex_prefix = prefix + ".vertex." + std::to_string(i);
      Add(signature, vertex_prefix + ".position", Vec3Text(vertex.position, options.stable_float_format));
      Add(signature, vertex_prefix + ".normal", Vec3Text(vertex.normal, options.stable_float_format));
      Add(signature, vertex_prefix + ".uv", Vec2Text(vertex.texcoord, options.stable_float_format));
    }
    for (std::size_t i = 0; i < mesh.indices.size(); ++i) {
      Add(signature, prefix + ".index." + std::to_string(i), std::to_string(mesh.indices[i]));
    }
  }
}

void AddCameras(SceneSignature* signature, const Scene& scene, const SignatureOptions& options) {
  AddCount(signature, "cameras.count", scene.cameras.size());
  std::vector<Camera> cameras = scene.cameras;
  std::sort(cameras.begin(), cameras.end(), [](const Camera& left, const Camera& right) {
    return left.id < right.id;
  });
  for (const auto& camera : cameras) {
    const std::string prefix = "camera." + Id(camera.id);
    if (options.include_names) Add(signature, prefix + ".name", camera.name);
    Add(signature, prefix + ".fov", FloatText(camera.vertical_fov_degrees, options.stable_float_format));
    Add(signature, prefix + ".near", FloatText(camera.near_plane, options.stable_float_format));
    Add(signature, prefix + ".far", FloatText(camera.far_plane, options.stable_float_format));
  }
}

void AddLights(SceneSignature* signature, const Scene& scene, const SignatureOptions& options) {
  AddCount(signature, "lights.count", scene.lights.size());
  std::vector<Light> lights = scene.lights;
  std::sort(lights.begin(), lights.end(), [](const Light& left, const Light& right) {
    return left.id < right.id;
  });
  for (const auto& light : lights) {
    const std::string prefix = "light." + Id(light.id);
    if (options.include_names) Add(signature, prefix + ".name", light.name);
    Add(signature, prefix + ".type", std::to_string(static_cast<int>(light.type)));
    Add(signature, prefix + ".color", Vec3Text(light.color, options.stable_float_format));
    Add(signature, prefix + ".intensity", FloatText(light.intensity, options.stable_float_format));
  }
}

void AddSkeletons(SceneSignature* signature, const Scene& scene, const SignatureOptions& options) {
  AddCount(signature, "skeletons.count", scene.skeletons.size());
  std::vector<Skeleton> skeletons = scene.skeletons;
  std::sort(skeletons.begin(), skeletons.end(), [](const Skeleton& left, const Skeleton& right) {
    return left.id < right.id;
  });
  for (const auto& skeleton : skeletons) {
    const std::string prefix = "skeleton." + Id(skeleton.id);
    if (options.include_names) Add(signature, prefix + ".name", skeleton.name);
    AddCount(signature, prefix + ".joints.count", skeleton.joints.size());
    std::vector<SkeletonJoint> joints = skeleton.joints;
    std::sort(joints.begin(), joints.end(), [](const SkeletonJoint& left, const SkeletonJoint& right) {
      return left.id < right.id;
    });
    for (const auto& joint : joints) {
      const std::string joint_prefix = prefix + ".joint." + Id(joint.id);
      if (options.include_names) Add(signature, joint_prefix + ".name", joint.name);
      Add(signature, joint_prefix + ".parent", Id(joint.parent));
      Add(signature, joint_prefix + ".bind_pose", TransformText(joint.bind_pose, options.stable_float_format));
    }
  }
}

void AddAnimations(SceneSignature* signature, const Scene& scene, const SignatureOptions& options) {
  AddCount(signature, "animations.count", scene.animations.size());
  if (!options.include_animation) return;
  std::vector<Animation> animations = scene.animations;
  std::sort(animations.begin(), animations.end(), [](const Animation& left, const Animation& right) {
    return left.id < right.id;
  });
  for (const auto& animation : animations) {
    const std::string prefix = "animation." + Id(animation.id);
    if (options.include_names) Add(signature, prefix + ".name", animation.name);
    Add(signature, prefix + ".duration", FloatText(animation.duration_seconds, options.stable_float_format));
    AddCount(signature, prefix + ".channels.count", animation.channels.size());
    for (std::size_t channel_index = 0; channel_index < animation.channels.size(); ++channel_index) {
      const auto& channel = animation.channels[channel_index];
      const std::string channel_prefix = prefix + ".channel." + std::to_string(channel_index);
      Add(signature, channel_prefix + ".target", Id(channel.target_node));
      AddCount(signature, channel_prefix + ".keys.count", channel.keys.size());
      for (std::size_t key_index = 0; key_index < channel.keys.size(); ++key_index) {
        const auto& key = channel.keys[key_index];
        const std::string key_prefix = channel_prefix + ".key." + std::to_string(key_index);
        Add(signature, key_prefix + ".time", FloatText(key.time_seconds, options.stable_float_format));
        Add(signature, key_prefix + ".transform", TransformText(key.transform, options.stable_float_format));
      }
    }
  }
}

std::uint64_t HashEntries(const std::vector<SignatureEntry>& entries) {
  std::uint64_t hash = kFnvOffset;
  for (const auto& entry : entries) {
    hash = HashBytes(hash, entry.path);
    hash = HashBytes(hash, "=");
    hash = HashBytes(hash, entry.value);
    hash = HashBytes(hash, "\n");
  }
  return hash;
}

}  // namespace

SceneSignature ComputeSceneSignature(const Scene& scene, const SignatureOptions& options) {
  SceneSignature signature;
  Add(&signature, "scene.name", options.include_names ? scene.name : "<name-omitted>");
  if (options.include_metadata) AddMetadata(&signature, scene);
  AddNodes(&signature, scene, options);
  AddTextures(&signature, scene, options);
  AddMaterials(&signature, scene, options);
  AddMeshes(&signature, scene, options);
  AddCameras(&signature, scene, options);
  AddLights(&signature, scene, options);
  AddSkeletons(&signature, scene, options);
  AddAnimations(&signature, scene, options);
  std::sort(signature.entries.begin(), signature.entries.end(),
            [](const SignatureEntry& left, const SignatureEntry& right) {
              if (left.path != right.path) return left.path < right.path;
              return left.value < right.value;
            });
  signature.hash = HashEntries(signature.entries);
  return signature;
}

std::string FormatSceneSignature(const SceneSignature& signature) {
  std::ostringstream out;
  out << SceneSignatureDigest(signature) << "\n";
  for (const auto& entry : signature.entries) {
    out << entry.path << "=" << entry.value << "\n";
  }
  return out.str();
}

std::string SceneSignatureDigest(const SceneSignature& signature) {
  std::ostringstream out;
  out << std::hex << std::setw(16) << std::setfill('0') << signature.hash;
  return out.str();
}

bool SceneSignaturesEquivalent(const SceneSignature& left, const SceneSignature& right) {
  if (left.hash != right.hash) return false;
  if (left.entries.size() != right.entries.size()) return false;
  for (std::size_t i = 0; i < left.entries.size(); ++i) {
    if (left.entries[i].path != right.entries[i].path) return false;
    if (left.entries[i].value != right.entries[i].value) return false;
  }
  return true;
}

SignatureDiff DiffSceneSignatures(const SceneSignature& left, const SceneSignature& right) {
  SignatureDiff diff;
  diff.left_digest = SceneSignatureDigest(left);
  diff.right_digest = SceneSignatureDigest(right);
  std::map<std::string, std::string> left_entries;
  std::map<std::string, std::string> right_entries;
  for (const auto& entry : left.entries) left_entries[entry.path] = entry.value;
  for (const auto& entry : right.entries) right_entries[entry.path] = entry.value;

  for (const auto& entry : left_entries) {
    auto found = right_entries.find(entry.first);
    if (found == right_entries.end()) {
      diff.deltas.push_back({SignatureDeltaKind::kRemoved, entry.first, entry.second, ""});
    } else if (found->second != entry.second) {
      diff.deltas.push_back({SignatureDeltaKind::kChanged, entry.first, entry.second, found->second});
    }
  }
  for (const auto& entry : right_entries) {
    if (left_entries.find(entry.first) == left_entries.end()) {
      diff.deltas.push_back({SignatureDeltaKind::kAdded, entry.first, "", entry.second});
    }
  }
  std::sort(diff.deltas.begin(), diff.deltas.end(), [](const SignatureDelta& left_delta,
                                                       const SignatureDelta& right_delta) {
    if (left_delta.path != right_delta.path) return left_delta.path < right_delta.path;
    return static_cast<int>(left_delta.kind) < static_cast<int>(right_delta.kind);
  });
  return diff;
}

SignatureDiffSummary SummarizeSignatureDiff(const SignatureDiff& diff) {
  SignatureDiffSummary summary;
  for (const auto& delta : diff.deltas) {
    switch (delta.kind) {
      case SignatureDeltaKind::kAdded:
        ++summary.added;
        break;
      case SignatureDeltaKind::kRemoved:
        ++summary.removed;
        break;
      case SignatureDeltaKind::kChanged:
        ++summary.changed;
        break;
    }
    if (delta.path.find("node.") == 0 || delta.path.find("nodes.") == 0) {
      ++summary.node_changes;
    } else if (delta.path.find("mesh.") == 0 || delta.path.find("meshes.") == 0) {
      ++summary.mesh_changes;
    } else if (delta.path.find("material.") == 0 || delta.path.find("materials.") == 0) {
      ++summary.material_changes;
    } else if (delta.path.find("texture.") == 0 || delta.path.find("textures.") == 0) {
      ++summary.texture_changes;
    } else if (delta.path.find("animation.") == 0 || delta.path.find("animations.") == 0) {
      ++summary.animation_changes;
    } else if (delta.path.find("metadata.") == 0) {
      ++summary.metadata_changes;
    }
  }
  return summary;
}

std::string SignatureDeltaKindName(SignatureDeltaKind kind) {
  switch (kind) {
    case SignatureDeltaKind::kAdded:
      return "added";
    case SignatureDeltaKind::kRemoved:
      return "removed";
    case SignatureDeltaKind::kChanged:
      return "changed";
  }
  return "unknown";
}

std::string FormatSignatureDiffSummary(const SignatureDiffSummary& summary) {
  std::ostringstream out;
  out << "added=" << summary.added << "\n";
  out << "removed=" << summary.removed << "\n";
  out << "changed=" << summary.changed << "\n";
  out << "node_changes=" << summary.node_changes << "\n";
  out << "mesh_changes=" << summary.mesh_changes << "\n";
  out << "material_changes=" << summary.material_changes << "\n";
  out << "texture_changes=" << summary.texture_changes << "\n";
  out << "animation_changes=" << summary.animation_changes << "\n";
  out << "metadata_changes=" << summary.metadata_changes << "\n";
  return out.str();
}

std::string FormatSignatureDiff(const SignatureDiff& diff) {
  std::ostringstream out;
  out << "left=" << diff.left_digest << "\n";
  out << "right=" << diff.right_digest << "\n";
  out << FormatSignatureDiffSummary(SummarizeSignatureDiff(diff));
  for (const auto& delta : diff.deltas) {
    out << SignatureDeltaKindName(delta.kind) << " " << delta.path;
    if (delta.kind == SignatureDeltaKind::kAdded) {
      out << " +" << delta.right_value;
    } else if (delta.kind == SignatureDeltaKind::kRemoved) {
      out << " -" << delta.left_value;
    } else {
      out << " " << delta.left_value << " -> " << delta.right_value;
    }
    out << "\n";
  }
  return out.str();
}

bool SceneSignatureIndex::Add(std::string name, const SceneSignature& signature) {
  const std::string digest = SceneSignatureDigest(signature);
  for (const auto& entry : entries_) {
    if (entry.name == name) return false;
  }
  entries_.push_back({std::move(name), digest, signature.entries.size()});
  std::sort(entries_.begin(), entries_.end(), [](const SignatureIndexEntry& left,
                                                const SignatureIndexEntry& right) {
    if (left.digest != right.digest) return left.digest < right.digest;
    return left.name < right.name;
  });
  return true;
}

bool SceneSignatureIndex::Remove(const std::string& name) {
  const auto before = entries_.size();
  entries_.erase(std::remove_if(entries_.begin(), entries_.end(),
                                [&name](const SignatureIndexEntry& entry) {
                                  return entry.name == name;
                                }),
                 entries_.end());
  return entries_.size() != before;
}

void SceneSignatureIndex::Clear() {
  entries_.clear();
}

bool SceneSignatureIndex::ContainsDigest(const std::string& digest) const {
  return std::any_of(entries_.begin(), entries_.end(), [&digest](const SignatureIndexEntry& entry) {
    return entry.digest == digest;
  });
}

bool SceneSignatureIndex::ContainsName(const std::string& name) const {
  return std::any_of(entries_.begin(), entries_.end(), [&name](const SignatureIndexEntry& entry) {
    return entry.name == name;
  });
}

std::string SceneSignatureIndex::DigestForName(const std::string& name) const {
  for (const auto& entry : entries_) {
    if (entry.name == name) return entry.digest;
  }
  return {};
}

std::vector<std::string> SceneSignatureIndex::NamesForDigest(const std::string& digest) const {
  std::vector<std::string> names;
  for (const auto& entry : entries_) {
    if (entry.digest == digest) names.push_back(entry.name);
  }
  return names;
}

std::vector<SignatureIndexEntry> SceneSignatureIndex::UniqueEntries() const {
  std::vector<SignatureIndexEntry> unique;
  std::unordered_set<std::string> seen;
  for (const auto& entry : entries_) {
    if (seen.insert(entry.digest).second) unique.push_back(entry);
  }
  return unique;
}

std::vector<SignatureIndexEntry> SceneSignatureIndex::Entries() const {
  return entries_;
}

std::vector<std::vector<std::string>> SceneSignatureIndex::DuplicateGroups() const {
  std::vector<std::vector<std::string>> groups;
  std::string current_digest;
  std::vector<std::string> current_group;
  for (const auto& entry : entries_) {
    if (current_digest.empty()) current_digest = entry.digest;
    if (entry.digest != current_digest) {
      if (current_group.size() > 1U) groups.push_back(current_group);
      current_group.clear();
      current_digest = entry.digest;
    }
    current_group.push_back(entry.name);
  }
  if (current_group.size() > 1U) groups.push_back(current_group);
  return groups;
}

std::vector<std::string> SceneSignatureIndex::DuplicateDigests() const {
  std::vector<std::string> digests;
  std::string current_digest;
  std::size_t current_count = 0;
  auto flush = [&digests, &current_digest, &current_count]() {
    if (!current_digest.empty() && current_count > 1U) digests.push_back(current_digest);
  };
  for (const auto& entry : entries_) {
    if (current_digest.empty()) {
      current_digest = entry.digest;
      current_count = 1;
      continue;
    }
    if (entry.digest == current_digest) {
      ++current_count;
      continue;
    }
    flush();
    current_digest = entry.digest;
    current_count = 1;
  }
  flush();
  return digests;
}

std::string SceneSignatureIndex::FirstDuplicateDigest() const {
  const auto digests = DuplicateDigests();
  return digests.empty() ? std::string{} : digests.front();
}

SignatureIndexStats SceneSignatureIndex::Stats() const {
  SignatureIndexStats stats;
  stats.entries = entries_.size();
  stats.unique_digests = UniqueEntries().size();
  const auto groups = DuplicateGroups();
  stats.duplicate_digests = groups.size();
  for (const auto& group : groups) {
    stats.duplicate_entries += group.size();
    stats.largest_group = std::max(stats.largest_group, group.size());
  }
  return stats;
}

bool SceneSignatureIndex::HasDuplicates() const {
  std::string previous_digest;
  bool have_previous = false;
  for (const auto& entry : entries_) {
    if (have_previous && entry.digest == previous_digest) {
      return true;
    }
    previous_digest = entry.digest;
    have_previous = true;
  }
  return false;
}

bool SceneSignatureIndex::IsUnique() const {
  return !HasDuplicates();
}

std::size_t SceneSignatureIndex::DuplicateEntryCount() const {
  std::size_t count = 0;
  for (const auto& group : DuplicateGroups()) {
    count += group.size();
  }
  return count;
}

std::size_t SceneSignatureIndex::DigestCount(const std::string& digest) const {
  return static_cast<std::size_t>(std::count_if(entries_.begin(), entries_.end(),
                                               [&digest](const SignatureIndexEntry& entry) {
                                                 return entry.digest == digest;
                                               }));
}

std::string FormatSignatureIndexStats(const SignatureIndexStats& stats) {
  std::ostringstream out;
  out << "entries=" << stats.entries << "\n";
  out << "unique_digests=" << stats.unique_digests << "\n";
  out << "duplicate_digests=" << stats.duplicate_digests << "\n";
  out << "duplicate_entries=" << stats.duplicate_entries << "\n";
  out << "largest_group=" << stats.largest_group << "\n";
  out << "has_duplicates=" << (stats.duplicate_digests == 0 ? "false" : "true") << "\n";
  return out.str();
}

std::string FormatSignatureIndex(const SceneSignatureIndex& index) {
  std::ostringstream out;
  out << FormatSignatureIndexStats(index.Stats());
  if (!index.FirstDuplicateDigest().empty()) {
    out << "first_duplicate_digest=" << index.FirstDuplicateDigest() << "\n";
  }
  for (const auto& entry : index.Entries()) {
    out << entry.digest << " " << entry.name << " entries=" << entry.entry_count
        << " digest_count=" << index.DigestCount(entry.digest) << "\n";
  }
  const auto duplicates = index.DuplicateGroups();
  out << "duplicate_groups=" << duplicates.size() << "\n";
  for (const auto& group : duplicates) {
    out << "duplicate";
    for (const auto& name : group) out << " " << name;
    out << "\n";
  }
  return out.str();
}

}  // namespace bse
