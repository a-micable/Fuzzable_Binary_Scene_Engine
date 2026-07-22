#include "bse/obj_io.hpp"

#include "bse/mesh_tools.hpp"
#include "bse/scene_stats.hpp"

#include <cstdlib>
#include <sstream>
#include <unordered_map>

namespace bse {

namespace {

std::string Trim(std::string value) {
  const auto first = value.find_first_not_of(" \t\r\n");
  if (first == std::string::npos) return {};
  const auto last = value.find_last_not_of(" \t\r\n");
  return value.substr(first, last - first + 1U);
}

std::vector<std::string> Split(const std::string& value, char delimiter) {
  std::vector<std::string> parts;
  std::string current;
  std::istringstream in(value);
  while (std::getline(in, current, delimiter)) {
    parts.push_back(current);
  }
  return parts;
}

Result<float> ParseFloat(const std::string& value) {
  char* end = nullptr;
  const float parsed = std::strtof(value.c_str(), &end);
  if (end == value.c_str() || *end != '\0') {
    return Status::Error(ErrorCode::kMalformedData, "invalid OBJ float: " + value);
  }
  return parsed;
}

Result<int> ParseIndex(const std::string& value) {
  char* end = nullptr;
  const long parsed = std::strtol(value.c_str(), &end, 10);
  if (end == value.c_str() || *end != '\0') {
    return Status::Error(ErrorCode::kMalformedData, "invalid OBJ index: " + value);
  }
  return static_cast<int>(parsed);
}

std::uint32_t ResolveObjIndex(int index, std::size_t count) {
  if (index > 0) {
    return static_cast<std::uint32_t>(index - 1);
  }
  return static_cast<std::uint32_t>(static_cast<int>(count) + index);
}

struct ObjVertexRef {
  std::uint32_t position = 0;
  std::uint32_t texcoord = UINT32_MAX;
  std::uint32_t normal = UINT32_MAX;
};

Result<ObjVertexRef> ParseVertexRef(const std::string& token, std::size_t positions,
                                    std::size_t texcoords, std::size_t normals) {
  auto parts = Split(token, '/');
  if (parts.empty() || parts.size() > 3) {
    return Status::Error(ErrorCode::kMalformedData, "invalid OBJ face token: " + token);
  }
  auto position = ParseIndex(parts[0]);
  if (!position.ok()) return position.status();
  ObjVertexRef ref;
  ref.position = ResolveObjIndex(position.value(), positions);
  if (parts.size() >= 2 && !parts[1].empty()) {
    auto uv = ParseIndex(parts[1]);
    if (!uv.ok()) return uv.status();
    ref.texcoord = ResolveObjIndex(uv.value(), texcoords);
  }
  if (parts.size() >= 3 && !parts[2].empty()) {
    auto normal = ParseIndex(parts[2]);
    if (!normal.ok()) return normal.status();
    ref.normal = ResolveObjIndex(normal.value(), normals);
  }
  return ref;
}

}  // namespace

std::vector<std::string> SplitObjLines(const std::string& text) {
  std::vector<std::string> lines;
  std::string line;
  std::istringstream in(text);
  while (std::getline(in, line)) {
    lines.push_back(line);
  }
  return lines;
}

Result<std::string> WriteObjMesh(const Mesh& mesh, const ObjWriteOptions& options) {
  auto status = ValidateMeshTopology(mesh);
  if (!status.ok()) {
    return status.status();
  }
  std::ostringstream out;
  if (options.include_object_names) {
    out << "o " << mesh.name << "\n";
  }
  for (const auto& vertex : mesh.vertices) {
    out << "v " << vertex.position.x << " " << vertex.position.y << " " << vertex.position.z
        << "\n";
  }
  if (options.include_texcoords) {
    for (const auto& vertex : mesh.vertices) {
      out << "vt " << vertex.texcoord.x << " " << vertex.texcoord.y << "\n";
    }
  }
  if (options.include_normals) {
    for (const auto& vertex : mesh.vertices) {
      out << "vn " << vertex.normal.x << " " << vertex.normal.y << " " << vertex.normal.z
          << "\n";
    }
  }
  for (std::size_t i = 0; i + 2U < mesh.indices.size(); i += 3U) {
    out << "f";
    for (std::size_t j = 0; j < 3; ++j) {
      const auto index = mesh.indices[i + j] + 1U;
      out << " " << index;
      if (options.include_texcoords || options.include_normals) {
        out << "/";
        if (options.include_texcoords) {
          out << index;
        }
        if (options.include_normals) {
          out << "/" << index;
        }
      }
    }
    out << "\n";
  }
  return out.str();
}

Result<std::string> WriteObjScene(const Scene& scene, const ObjWriteOptions& options) {
  std::ostringstream out;
  for (const auto& mesh : scene.meshes) {
    auto obj = WriteObjMesh(mesh, options);
    if (!obj.ok()) {
      return obj.status();
    }
    out << obj.value();
  }
  return out.str();
}

Result<Mesh> ReadObjMesh(const std::string& text, const ObjReadOptions& options) {
  std::vector<Vec3> positions;
  std::vector<Vec3> normals;
  std::vector<Vec2> texcoords;
  Mesh mesh;
  mesh.id = options.mesh_id;
  mesh.name = options.mesh_name;
  mesh.material = options.material_id;
  std::unordered_map<std::string, std::uint32_t> vertex_cache;

  for (auto line : SplitObjLines(text)) {
    const auto comment = line.find('#');
    if (comment != std::string::npos) {
      line = line.substr(0, comment);
    }
    line = Trim(std::move(line));
    if (line.empty()) {
      continue;
    }
    auto parts = Split(line, ' ');
    std::vector<std::string> tokens;
    for (auto part : parts) {
      part = Trim(part);
      if (!part.empty()) {
        tokens.push_back(part);
      }
    }
    if (tokens.empty()) {
      continue;
    }
    if (tokens[0] == "o" && tokens.size() >= 2) {
      mesh.name = tokens[1];
    } else if (tokens[0] == "v" && tokens.size() >= 4) {
      auto x = ParseFloat(tokens[1]);
      auto y = ParseFloat(tokens[2]);
      auto z = ParseFloat(tokens[3]);
      if (!x.ok()) return x.status();
      if (!y.ok()) return y.status();
      if (!z.ok()) return z.status();
      positions.push_back({x.value(), y.value(), z.value()});
    } else if (tokens[0] == "vn" && tokens.size() >= 4) {
      auto x = ParseFloat(tokens[1]);
      auto y = ParseFloat(tokens[2]);
      auto z = ParseFloat(tokens[3]);
      if (!x.ok()) return x.status();
      if (!y.ok()) return y.status();
      if (!z.ok()) return z.status();
      normals.push_back({x.value(), y.value(), z.value()});
    } else if (tokens[0] == "vt" && tokens.size() >= 3) {
      auto u = ParseFloat(tokens[1]);
      auto v = ParseFloat(tokens[2]);
      if (!u.ok()) return u.status();
      if (!v.ok()) return v.status();
      texcoords.push_back({u.value(), v.value()});
    } else if (tokens[0] == "f" && tokens.size() >= 4) {
      std::vector<std::uint32_t> polygon;
      for (std::size_t i = 1; i < tokens.size(); ++i) {
        auto ref = ParseVertexRef(tokens[i], positions.size(), texcoords.size(), normals.size());
        if (!ref.ok()) return ref.status();
        if (ref.value().position >= positions.size()) {
          return Status::Error(ErrorCode::kMalformedData, "OBJ face position is out of range");
        }
        auto cached = vertex_cache.find(tokens[i]);
        if (cached != vertex_cache.end()) {
          polygon.push_back(cached->second);
          continue;
        }
        Vertex vertex;
        vertex.position = positions[ref.value().position];
        if (ref.value().normal != UINT32_MAX && ref.value().normal < normals.size()) {
          vertex.normal = normals[ref.value().normal];
        }
        if (ref.value().texcoord != UINT32_MAX && ref.value().texcoord < texcoords.size()) {
          vertex.texcoord = texcoords[ref.value().texcoord];
        }
        const auto new_index = static_cast<std::uint32_t>(mesh.vertices.size());
        mesh.vertices.push_back(vertex);
        vertex_cache.emplace(tokens[i], new_index);
        polygon.push_back(new_index);
      }
      for (std::size_t i = 1; i + 1U < polygon.size(); ++i) {
        mesh.indices.push_back(polygon[0]);
        mesh.indices.push_back(polygon[i]);
        mesh.indices.push_back(polygon[i + 1U]);
      }
    }
  }
  if (mesh.vertices.empty()) {
    return Status::Error(ErrorCode::kMalformedData, "OBJ mesh has no vertices");
  }
  if (mesh.indices.empty()) {
    return Status::Error(ErrorCode::kMalformedData, "OBJ mesh has no faces");
  }
  RecalculateNormals(&mesh);
  auto status = ValidateMeshTopology(mesh);
  if (!status.ok()) {
    return status.status();
  }
  return mesh;
}

}  // namespace bse
