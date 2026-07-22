#include "bse/scene_diff.hpp"

#include <algorithm>
#include <sstream>
#include <unordered_map>

namespace bse {

namespace {

template <typename T>
std::unordered_map<ObjectId, const T*> IndexById(const std::vector<T>& values) {
  std::unordered_map<ObjectId, const T*> index;
  index.reserve(values.size());
  for (const auto& value : values) {
    index.emplace(value.id, &value);
  }
  return index;
}

std::string IdPath(const std::string& prefix, ObjectId id) {
  return prefix + "[" + std::to_string(id) + "]";
}

std::string F(float value) {
  std::ostringstream out;
  out.setf(std::ios::fixed);
  out.precision(4);
  out << value;
  return out.str();
}

std::string V2(const Vec2& value) {
  return F(value.x) + "," + F(value.y);
}

std::string V3(const Vec3& value) {
  return F(value.x) + "," + F(value.y) + "," + F(value.z);
}

std::string Q(const Quat& value) {
  return F(value.x) + "," + F(value.y) + "," + F(value.z) + "," + F(value.w);
}

void Add(SceneDiff* diff, DiffSeverity severity, std::string path, std::string message,
         std::string left, std::string right) {
  diff->entries.push_back({severity, std::move(path), std::move(message), std::move(left),
                           std::move(right)});
}

void CompareString(SceneDiff* diff, DiffSeverity severity, const std::string& path,
                   const std::string& label, const std::string& left, const std::string& right) {
  if (left != right) {
    Add(diff, severity, path + "." + label, label + " changed", left, right);
  }
}

void CompareId(SceneDiff* diff, DiffSeverity severity, const std::string& path,
               const std::string& label, ObjectId left, ObjectId right) {
  if (left != right) {
    Add(diff, severity, path + "." + label, label + " changed", std::to_string(left),
        std::to_string(right));
  }
}

void CompareFloat(SceneDiff* diff, DiffSeverity severity, const std::string& path,
                  const std::string& label, float left, float right) {
  if (left != right) {
    Add(diff, severity, path + "." + label, label + " changed", F(left), F(right));
  }
}

void CompareVec3(SceneDiff* diff, DiffSeverity severity, const std::string& path,
                 const std::string& label, const Vec3& left, const Vec3& right) {
  if (left.x != right.x || left.y != right.y || left.z != right.z) {
    Add(diff, severity, path + "." + label, label + " changed", V3(left), V3(right));
  }
}

void CompareTransform(SceneDiff* diff, const std::string& path, const Transform& left,
                      const Transform& right) {
  CompareVec3(diff, DiffSeverity::kWarning, path, "translation", left.translation,
              right.translation);
  if (left.rotation.x != right.rotation.x || left.rotation.y != right.rotation.y ||
      left.rotation.z != right.rotation.z || left.rotation.w != right.rotation.w) {
    Add(diff, DiffSeverity::kWarning, path + ".rotation", "rotation changed", Q(left.rotation),
        Q(right.rotation));
  }
  CompareVec3(diff, DiffSeverity::kWarning, path, "scale", left.scale, right.scale);
}

template <typename T>
void CompareMembership(SceneDiff* diff, const std::string& label, const std::vector<T>& left,
                       const std::vector<T>& right) {
  const auto left_index = IndexById(left);
  const auto right_index = IndexById(right);
  for (const auto& entry : left_index) {
    if (right_index.find(entry.first) == right_index.end()) {
      Add(diff, DiffSeverity::kBreaking, IdPath(label, entry.first), label + " removed",
          std::to_string(entry.first), "");
    }
  }
  for (const auto& entry : right_index) {
    if (left_index.find(entry.first) == left_index.end()) {
      Add(diff, DiffSeverity::kInfo, IdPath(label, entry.first), label + " added", "",
          std::to_string(entry.first));
    }
  }
}

void DiffNodes(SceneDiff* diff, const Scene& left, const Scene& right) {
  CompareMembership(diff, "nodes", left.nodes, right.nodes);
  const auto left_index = IndexById(left.nodes);
  const auto right_index = IndexById(right.nodes);
  for (const auto& entry : left_index) {
    auto it = right_index.find(entry.first);
    if (it == right_index.end()) {
      continue;
    }
    const auto& lhs = *entry.second;
    const auto& rhs = *it->second;
    const auto path = IdPath("nodes", lhs.id);
    CompareString(diff, DiffSeverity::kInfo, path, "name", lhs.name, rhs.name);
    CompareId(diff, DiffSeverity::kBreaking, path, "parent", lhs.parent, rhs.parent);
    CompareId(diff, DiffSeverity::kBreaking, path, "mesh", lhs.mesh, rhs.mesh);
    CompareId(diff, DiffSeverity::kBreaking, path, "camera", lhs.camera, rhs.camera);
    CompareId(diff, DiffSeverity::kBreaking, path, "light", lhs.light, rhs.light);
    CompareTransform(diff, path, lhs.local, rhs.local);
    if (lhs.children != rhs.children) {
      Add(diff, DiffSeverity::kWarning, path + ".children", "child list changed",
          std::to_string(lhs.children.size()), std::to_string(rhs.children.size()));
    }
  }
}

void DiffMeshes(SceneDiff* diff, const Scene& left, const Scene& right) {
  CompareMembership(diff, "meshes", left.meshes, right.meshes);
  const auto left_index = IndexById(left.meshes);
  const auto right_index = IndexById(right.meshes);
  for (const auto& entry : left_index) {
    auto it = right_index.find(entry.first);
    if (it == right_index.end()) {
      continue;
    }
    const auto& lhs = *entry.second;
    const auto& rhs = *it->second;
    const auto path = IdPath("meshes", lhs.id);
    CompareString(diff, DiffSeverity::kInfo, path, "name", lhs.name, rhs.name);
    CompareId(diff, DiffSeverity::kBreaking, path, "material", lhs.material, rhs.material);
    if (lhs.vertices.size() != rhs.vertices.size()) {
      Add(diff, DiffSeverity::kBreaking, path + ".vertices", "vertex count changed",
          std::to_string(lhs.vertices.size()), std::to_string(rhs.vertices.size()));
    } else {
      for (std::size_t i = 0; i < lhs.vertices.size(); ++i) {
        const auto vertex_path = path + ".vertices[" + std::to_string(i) + "]";
        CompareVec3(diff, DiffSeverity::kWarning, vertex_path, "position", lhs.vertices[i].position,
                    rhs.vertices[i].position);
        CompareVec3(diff, DiffSeverity::kInfo, vertex_path, "normal", lhs.vertices[i].normal,
                    rhs.vertices[i].normal);
        if (lhs.vertices[i].texcoord.x != rhs.vertices[i].texcoord.x ||
            lhs.vertices[i].texcoord.y != rhs.vertices[i].texcoord.y) {
          Add(diff, DiffSeverity::kInfo, vertex_path + ".uv", "texture coordinate changed",
              V2(lhs.vertices[i].texcoord), V2(rhs.vertices[i].texcoord));
        }
      }
    }
    if (lhs.indices != rhs.indices) {
      Add(diff, DiffSeverity::kBreaking, path + ".indices", "index buffer changed",
          std::to_string(lhs.indices.size()), std::to_string(rhs.indices.size()));
    }
  }
}

void DiffMaterials(SceneDiff* diff, const Scene& left, const Scene& right) {
  CompareMembership(diff, "materials", left.materials, right.materials);
  const auto left_index = IndexById(left.materials);
  const auto right_index = IndexById(right.materials);
  for (const auto& entry : left_index) {
    auto it = right_index.find(entry.first);
    if (it == right_index.end()) {
      continue;
    }
    const auto& lhs = *entry.second;
    const auto& rhs = *it->second;
    const auto path = IdPath("materials", lhs.id);
    CompareString(diff, DiffSeverity::kInfo, path, "name", lhs.name, rhs.name);
    CompareVec3(diff, DiffSeverity::kWarning, path, "base_color", lhs.base_color, rhs.base_color);
    CompareFloat(diff, DiffSeverity::kWarning, path, "roughness", lhs.roughness, rhs.roughness);
    CompareFloat(diff, DiffSeverity::kWarning, path, "metallic", lhs.metallic, rhs.metallic);
    CompareId(diff, DiffSeverity::kBreaking, path, "base_color_texture", lhs.base_color_texture,
              rhs.base_color_texture);
  }
}

void DiffTextures(SceneDiff* diff, const Scene& left, const Scene& right) {
  CompareMembership(diff, "textures", left.textures, right.textures);
  const auto left_index = IndexById(left.textures);
  const auto right_index = IndexById(right.textures);
  for (const auto& entry : left_index) {
    auto it = right_index.find(entry.first);
    if (it == right_index.end()) {
      continue;
    }
    const auto& lhs = *entry.second;
    const auto& rhs = *it->second;
    const auto path = IdPath("textures", lhs.id);
    CompareString(diff, DiffSeverity::kInfo, path, "name", lhs.name, rhs.name);
    CompareString(diff, DiffSeverity::kWarning, path, "uri", lhs.uri, rhs.uri);
    if (lhs.width != rhs.width || lhs.height != rhs.height) {
      Add(diff, DiffSeverity::kWarning, path + ".size", "texture dimensions changed",
          std::to_string(lhs.width) + "x" + std::to_string(lhs.height),
          std::to_string(rhs.width) + "x" + std::to_string(rhs.height));
    }
    CompareString(diff, DiffSeverity::kInfo, path, "color_space", lhs.color_space,
                  rhs.color_space);
  }
}

void DiffCameras(SceneDiff* diff, const Scene& left, const Scene& right) {
  CompareMembership(diff, "cameras", left.cameras, right.cameras);
  const auto left_index = IndexById(left.cameras);
  const auto right_index = IndexById(right.cameras);
  for (const auto& entry : left_index) {
    auto it = right_index.find(entry.first);
    if (it == right_index.end()) continue;
    const auto& lhs = *entry.second;
    const auto& rhs = *it->second;
    const auto path = IdPath("cameras", lhs.id);
    CompareString(diff, DiffSeverity::kInfo, path, "name", lhs.name, rhs.name);
    CompareFloat(diff, DiffSeverity::kWarning, path, "fov", lhs.vertical_fov_degrees,
                 rhs.vertical_fov_degrees);
    CompareFloat(diff, DiffSeverity::kBreaking, path, "near", lhs.near_plane, rhs.near_plane);
    CompareFloat(diff, DiffSeverity::kBreaking, path, "far", lhs.far_plane, rhs.far_plane);
  }
}

void DiffLights(SceneDiff* diff, const Scene& left, const Scene& right) {
  CompareMembership(diff, "lights", left.lights, right.lights);
  const auto left_index = IndexById(left.lights);
  const auto right_index = IndexById(right.lights);
  for (const auto& entry : left_index) {
    auto it = right_index.find(entry.first);
    if (it == right_index.end()) continue;
    const auto& lhs = *entry.second;
    const auto& rhs = *it->second;
    const auto path = IdPath("lights", lhs.id);
    CompareString(diff, DiffSeverity::kInfo, path, "name", lhs.name, rhs.name);
    if (lhs.type != rhs.type) {
      Add(diff, DiffSeverity::kWarning, path + ".type", "light type changed",
          std::to_string(static_cast<int>(lhs.type)), std::to_string(static_cast<int>(rhs.type)));
    }
    CompareVec3(diff, DiffSeverity::kWarning, path, "color", lhs.color, rhs.color);
    CompareFloat(diff, DiffSeverity::kWarning, path, "intensity", lhs.intensity, rhs.intensity);
  }
}

void DiffMetadata(SceneDiff* diff, const Scene& left, const Scene& right) {
  for (const auto& entry : left.metadata) {
    auto it = right.metadata.find(entry.first);
    if (it == right.metadata.end()) {
      Add(diff, DiffSeverity::kInfo, "metadata." + entry.first, "metadata removed", entry.second,
          "");
    } else if (entry.second != it->second) {
      Add(diff, DiffSeverity::kInfo, "metadata." + entry.first, "metadata changed", entry.second,
          it->second);
    }
  }
  for (const auto& entry : right.metadata) {
    if (left.metadata.find(entry.first) == left.metadata.end()) {
      Add(diff, DiffSeverity::kInfo, "metadata." + entry.first, "metadata added", "",
          entry.second);
    }
  }
}

}  // namespace

std::string DiffSeverityName(DiffSeverity severity) {
  switch (severity) {
    case DiffSeverity::kInfo:
      return "info";
    case DiffSeverity::kWarning:
      return "warning";
    case DiffSeverity::kBreaking:
      return "breaking";
  }
  return "unknown";
}

SceneDiff DiffScenes(const Scene& left, const Scene& right) {
  SceneDiff diff;
  CompareString(&diff, DiffSeverity::kInfo, "scene", "name", left.name, right.name);
  DiffMetadata(&diff, left, right);
  DiffNodes(&diff, left, right);
  DiffMeshes(&diff, left, right);
  DiffMaterials(&diff, left, right);
  DiffTextures(&diff, left, right);
  DiffCameras(&diff, left, right);
  DiffLights(&diff, left, right);
  CompareMembership(&diff, "skeletons", left.skeletons, right.skeletons);
  CompareMembership(&diff, "animations", left.animations, right.animations);
  std::stable_sort(diff.entries.begin(), diff.entries.end(), [](const auto& lhs, const auto& rhs) {
    if (lhs.path != rhs.path) {
      return lhs.path < rhs.path;
    }
    return static_cast<int>(lhs.severity) > static_cast<int>(rhs.severity);
  });
  return diff;
}

std::string FormatDiff(const SceneDiff& diff) {
  std::ostringstream out;
  for (const auto& entry : diff.entries) {
    out << DiffSeverityName(entry.severity) << " " << entry.path << ": " << entry.message;
    if (!entry.left.empty() || !entry.right.empty()) {
      out << " (" << entry.left << " -> " << entry.right << ")";
    }
    out << "\n";
  }
  return out.str();
}

}  // namespace bse
