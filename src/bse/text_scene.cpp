#include "bse/text_scene.hpp"

#include "bse/scene_ops.hpp"
#include "bse/validator.hpp"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <sstream>
#include <unordered_map>

namespace bse {

namespace {

struct Line {
  std::size_t number = 0;
  std::string section;
  std::unordered_map<std::string, std::string> fields;
};

std::string Trim(std::string value) {
  auto is_space = [](unsigned char c) { return std::isspace(c) != 0; };
  value.erase(value.begin(), std::find_if(value.begin(), value.end(),
                                          [&](char c) {
                                            return !is_space(static_cast<unsigned char>(c));
                                          }));
  value.erase(std::find_if(value.rbegin(), value.rend(), [&](char c) {
                return !is_space(static_cast<unsigned char>(c));
              }).base(),
              value.end());
  return value;
}

std::vector<std::string> Split(const std::string& value, char delimiter) {
  std::vector<std::string> parts;
  std::string current;
  std::istringstream in(value);
  while (std::getline(in, current, delimiter)) {
    parts.push_back(Trim(current));
  }
  if (!value.empty() && value.back() == delimiter) {
    parts.emplace_back();
  }
  return parts;
}

std::vector<std::string> TokenizeFields(const std::string& text) {
  std::vector<std::string> tokens;
  std::string current;
  bool quoted = false;
  bool escaped = false;
  for (char c : text) {
    if (escaped) {
      current.push_back(c);
      escaped = false;
      continue;
    }
    if (c == '\\') {
      current.push_back(c);
      escaped = true;
      continue;
    }
    if (c == '"') {
      quoted = !quoted;
      current.push_back(c);
      continue;
    }
    if (!quoted && std::isspace(static_cast<unsigned char>(c)) != 0) {
      if (!current.empty()) {
        tokens.push_back(current);
        current.clear();
      }
      continue;
    }
    current.push_back(c);
  }
  if (!current.empty()) {
    tokens.push_back(current);
  }
  return tokens;
}

std::string Unquote(std::string value) {
  value = Trim(std::move(value));
  if (value.size() >= 2 && value.front() == '"' && value.back() == '"') {
    std::string out;
    for (std::size_t i = 1; i + 1 < value.size(); ++i) {
      if (value[i] == '\\' && i + 2 < value.size()) {
        ++i;
        switch (value[i]) {
          case 'n':
            out.push_back('\n');
            break;
          case 't':
            out.push_back('\t');
            break;
          case '\\':
          case '"':
            out.push_back(value[i]);
            break;
          default:
            out.push_back(value[i]);
            break;
        }
      } else {
        out.push_back(value[i]);
      }
    }
    return out;
  }
  return value;
}

std::string Quote(const std::string& value) {
  std::string out = "\"";
  for (char c : value) {
    switch (c) {
      case '\\':
        out += "\\\\";
        break;
      case '"':
        out += "\\\"";
        break;
      case '\n':
        out += "\\n";
        break;
      case '\t':
        out += "\\t";
        break;
      default:
        out.push_back(c);
        break;
    }
  }
  out.push_back('"');
  return out;
}

Result<ObjectId> ParseId(const Line& line, const std::string& key, ObjectId fallback = kInvalidObjectId) {
  auto it = line.fields.find(key);
  if (it == line.fields.end()) {
    return fallback;
  }
  char* end = nullptr;
  const auto value = std::strtoull(it->second.c_str(), &end, 10);
  if (end == it->second.c_str() || *end != '\0') {
    return Status::Error(ErrorCode::kMalformedData,
                         "line " + std::to_string(line.number) + " has invalid id field " + key);
  }
  return static_cast<ObjectId>(value);
}

Result<std::uint32_t> ParseU32(const Line& line, const std::string& key, std::uint32_t fallback = 0) {
  auto id = ParseId(line, key, fallback);
  if (!id.ok()) {
    return id.status();
  }
  if (id.value() > UINT32_MAX) {
    return Status::Error(ErrorCode::kMalformedData,
                         "line " + std::to_string(line.number) + " has u32 overflow in " + key);
  }
  return static_cast<std::uint32_t>(id.value());
}

Result<float> ParseFloat(const Line& line, const std::string& key, float fallback = 0.0F) {
  auto it = line.fields.find(key);
  if (it == line.fields.end()) {
    return fallback;
  }
  char* end = nullptr;
  const float value = std::strtof(it->second.c_str(), &end);
  if (end == it->second.c_str() || *end != '\0') {
    return Status::Error(ErrorCode::kMalformedData,
                         "line " + std::to_string(line.number) + " has invalid float field " + key);
  }
  return value;
}

Result<Vec3> ParseVec3(const Line& line, const std::string& key, Vec3 fallback = {}) {
  auto it = line.fields.find(key);
  if (it == line.fields.end()) {
    return fallback;
  }
  auto parts = Split(it->second, ',');
  if (parts.size() != 3) {
    return Status::Error(ErrorCode::kMalformedData,
                         "line " + std::to_string(line.number) + " expected vec3 for " + key);
  }
  Line synthetic = line;
  synthetic.fields["x"] = parts[0];
  synthetic.fields["y"] = parts[1];
  synthetic.fields["z"] = parts[2];
  auto x = ParseFloat(synthetic, "x");
  auto y = ParseFloat(synthetic, "y");
  auto z = ParseFloat(synthetic, "z");
  if (!x.ok()) return x.status();
  if (!y.ok()) return y.status();
  if (!z.ok()) return z.status();
  return Vec3{x.value(), y.value(), z.value()};
}

Result<Vec2> ParseVec2(const Line& line, const std::string& key, Vec2 fallback = {}) {
  auto it = line.fields.find(key);
  if (it == line.fields.end()) {
    return fallback;
  }
  auto parts = Split(it->second, ',');
  if (parts.size() != 2) {
    return Status::Error(ErrorCode::kMalformedData,
                         "line " + std::to_string(line.number) + " expected vec2 for " + key);
  }
  Line synthetic = line;
  synthetic.fields["x"] = parts[0];
  synthetic.fields["y"] = parts[1];
  auto x = ParseFloat(synthetic, "x");
  auto y = ParseFloat(synthetic, "y");
  if (!x.ok()) return x.status();
  if (!y.ok()) return y.status();
  return Vec2{x.value(), y.value()};
}

std::string Field(const Line& line, const std::string& key, const std::string& fallback = {}) {
  auto it = line.fields.find(key);
  return it == line.fields.end() ? fallback : Unquote(it->second);
}

Result<Line> ParseLine(std::string text, std::size_t number) {
  const auto comment = text.find('#');
  if (comment != std::string::npos) {
    text = text.substr(0, comment);
  }
  text = Trim(std::move(text));
  if (text.empty()) {
    return Line{number, "", {}};
  }
  auto tokens = TokenizeFields(text);
  if (tokens.empty()) {
    return Line{number, "", {}};
  }
  Line line;
  line.number = number;
  line.section = tokens.front();
  for (std::size_t i = 1; i < tokens.size(); ++i) {
    const auto equals = tokens[i].find('=');
    if (equals == std::string::npos) {
      return Status::Error(ErrorCode::kMalformedData,
                           "line " + std::to_string(number) + " has a field without '='");
    }
    auto key = Trim(tokens[i].substr(0, equals));
    auto value = Trim(tokens[i].substr(equals + 1));
    if (key.empty()) {
      return Status::Error(ErrorCode::kMalformedData,
                           "line " + std::to_string(number) + " has an empty field name");
    }
    line.fields[std::move(key)] = std::move(value);
  }
  return line;
}

Result<LightType> ParseLightType(const Line& line) {
  const auto value = Field(line, "type", "point");
  if (value == "directional") return LightType::kDirectional;
  if (value == "point") return LightType::kPoint;
  if (value == "spot") return LightType::kSpot;
  return Status::Error(ErrorCode::kMalformedData,
                       "line " + std::to_string(line.number) + " has invalid light type");
}

std::string LightTypeName(LightType type) {
  switch (type) {
    case LightType::kDirectional:
      return "directional";
    case LightType::kPoint:
      return "point";
    case LightType::kSpot:
      return "spot";
  }
  return "unknown";
}

void WriteTransformFields(std::ostream& out, const Transform& transform) {
  out << " translation=" << transform.translation.x << "," << transform.translation.y << ","
      << transform.translation.z;
  out << " scale=" << transform.scale.x << "," << transform.scale.y << "," << transform.scale.z;
}

Result<void*> UnsupportedSection(const Line& line) {
  return Status::Error(ErrorCode::kMalformedData,
                       "line " + std::to_string(line.number) + " has unknown section " +
                           line.section);
}

}  // namespace

std::vector<std::string> SplitTextSceneLines(const std::string& text) {
  std::vector<std::string> lines;
  std::string line;
  std::istringstream in(text);
  while (std::getline(in, line)) {
    lines.push_back(line);
  }
  return lines;
}

Result<Scene> ParseTextScene(const std::string& text) {
  return ParseTextSceneLines(SplitTextSceneLines(text));
}

Result<Scene> ParseTextSceneLines(const std::vector<std::string>& lines) {
  Scene scene;
  scene.name = "Untitled";
  std::unordered_map<ObjectId, std::vector<Vertex>> pending_vertices;
  std::unordered_map<ObjectId, std::vector<std::uint32_t>> pending_indices;

  for (std::size_t i = 0; i < lines.size(); ++i) {
    auto parsed = ParseLine(lines[i], i + 1U);
    if (!parsed.ok()) {
      return parsed.status();
    }
    Line line = std::move(parsed.value());
    if (line.section.empty()) {
      continue;
    }
    if (line.section == "scene") {
      scene.name = Field(line, "name", scene.name);
    } else if (line.section == "meta") {
      const auto key = Field(line, "key");
      if (key.empty()) {
        return Status::Error(ErrorCode::kMalformedData,
                             "line " + std::to_string(line.number) + " metadata key is empty");
      }
      scene.metadata[key] = Field(line, "value");
    } else if (line.section == "node") {
      Node node;
      auto id = ParseId(line, "id");
      auto parent = ParseId(line, "parent");
      auto mesh = ParseId(line, "mesh");
      auto camera = ParseId(line, "camera");
      auto light = ParseId(line, "light");
      auto translation = ParseVec3(line, "translation");
      auto scale = ParseVec3(line, "scale", {1.0F, 1.0F, 1.0F});
      if (!id.ok()) return id.status();
      if (!parent.ok()) return parent.status();
      if (!mesh.ok()) return mesh.status();
      if (!camera.ok()) return camera.status();
      if (!light.ok()) return light.status();
      if (!translation.ok()) return translation.status();
      if (!scale.ok()) return scale.status();
      node.id = id.value();
      node.name = Field(line, "name", "node");
      node.parent = parent.value();
      node.mesh = mesh.value();
      node.camera = camera.value();
      node.light = light.value();
      node.local.translation = translation.value();
      node.local.scale = scale.value();
      scene.nodes.push_back(std::move(node));
    } else if (line.section == "texture") {
      Texture texture;
      auto id = ParseId(line, "id");
      auto width = ParseU32(line, "width", 1);
      auto height = ParseU32(line, "height", 1);
      if (!id.ok()) return id.status();
      if (!width.ok()) return width.status();
      if (!height.ok()) return height.status();
      texture.id = id.value();
      texture.name = Field(line, "name", "texture");
      texture.uri = Field(line, "uri");
      texture.width = width.value();
      texture.height = height.value();
      texture.color_space = Field(line, "color", "linear");
      scene.textures.push_back(std::move(texture));
    } else if (line.section == "material") {
      Material material;
      auto id = ParseId(line, "id");
      auto color = ParseVec3(line, "base", {1.0F, 1.0F, 1.0F});
      auto roughness = ParseFloat(line, "roughness", 1.0F);
      auto metallic = ParseFloat(line, "metallic", 0.0F);
      auto texture = ParseId(line, "texture");
      if (!id.ok()) return id.status();
      if (!color.ok()) return color.status();
      if (!roughness.ok()) return roughness.status();
      if (!metallic.ok()) return metallic.status();
      if (!texture.ok()) return texture.status();
      material.id = id.value();
      material.name = Field(line, "name", "material");
      material.base_color = color.value();
      material.roughness = roughness.value();
      material.metallic = metallic.value();
      material.base_color_texture = texture.value();
      scene.materials.push_back(std::move(material));
    } else if (line.section == "mesh") {
      Mesh mesh;
      auto id = ParseId(line, "id");
      auto material = ParseId(line, "material");
      if (!id.ok()) return id.status();
      if (!material.ok()) return material.status();
      mesh.id = id.value();
      mesh.name = Field(line, "name", "mesh");
      mesh.material = material.value();
      scene.meshes.push_back(std::move(mesh));
    } else if (line.section == "vertex") {
      auto mesh = ParseId(line, "mesh");
      auto position = ParseVec3(line, "position");
      auto normal = ParseVec3(line, "normal", {0.0F, 1.0F, 0.0F});
      auto uv = ParseVec2(line, "uv");
      if (!mesh.ok()) return mesh.status();
      if (!position.ok()) return position.status();
      if (!normal.ok()) return normal.status();
      if (!uv.ok()) return uv.status();
      pending_vertices[mesh.value()].push_back({position.value(), normal.value(), uv.value()});
    } else if (line.section == "index") {
      auto mesh = ParseId(line, "mesh");
      auto value = ParseU32(line, "value");
      if (!mesh.ok()) return mesh.status();
      if (!value.ok()) return value.status();
      pending_indices[mesh.value()].push_back(value.value());
    } else if (line.section == "camera") {
      Camera camera;
      auto id = ParseId(line, "id");
      auto fov = ParseFloat(line, "fov", 60.0F);
      auto near_plane = ParseFloat(line, "near", 0.01F);
      auto far_plane = ParseFloat(line, "far", 1000.0F);
      if (!id.ok()) return id.status();
      if (!fov.ok()) return fov.status();
      if (!near_plane.ok()) return near_plane.status();
      if (!far_plane.ok()) return far_plane.status();
      camera.id = id.value();
      camera.name = Field(line, "name", "camera");
      camera.vertical_fov_degrees = fov.value();
      camera.near_plane = near_plane.value();
      camera.far_plane = far_plane.value();
      scene.cameras.push_back(std::move(camera));
    } else if (line.section == "light") {
      Light light;
      auto id = ParseId(line, "id");
      auto type = ParseLightType(line);
      auto color = ParseVec3(line, "color", {1.0F, 1.0F, 1.0F});
      auto intensity = ParseFloat(line, "intensity", 1.0F);
      if (!id.ok()) return id.status();
      if (!type.ok()) return type.status();
      if (!color.ok()) return color.status();
      if (!intensity.ok()) return intensity.status();
      light.id = id.value();
      light.name = Field(line, "name", "light");
      light.type = type.value();
      light.color = color.value();
      light.intensity = intensity.value();
      scene.lights.push_back(std::move(light));
    } else {
      auto unsupported = UnsupportedSection(line);
      return unsupported.status();
    }
  }

  for (auto& mesh : scene.meshes) {
    auto vertices = pending_vertices.find(mesh.id);
    if (vertices != pending_vertices.end()) {
      mesh.vertices = std::move(vertices->second);
    }
    auto indices = pending_indices.find(mesh.id);
    if (indices != pending_indices.end()) {
      mesh.indices = std::move(indices->second);
    }
  }
  NormalizeOptions options;
  options.drop_unreferenced_resources = false;
  NormalizeScene(&scene, options);
  auto status = ValidateScene(scene);
  if (!status.ok()) {
    return status;
  }
  return scene;
}

Result<std::string> WriteTextScene(const Scene& scene, const TextSceneOptions& options) {
  auto status = ValidateScene(scene);
  if (!status.ok()) {
    return status;
  }
  std::ostringstream out;
  if (options.include_comments) {
    out << "# Binary Scene Engine text scene\n";
  }
  out << "scene name=" << Quote(scene.name) << "\n";
  for (const auto& entry : scene.metadata) {
    out << "meta key=" << Quote(entry.first) << " value=" << Quote(entry.second) << "\n";
  }
  for (const auto& node : scene.nodes) {
    out << "node id=" << node.id << " name=" << Quote(node.name);
    if (node.parent != kInvalidObjectId) out << " parent=" << node.parent;
    if (node.mesh != kInvalidObjectId) out << " mesh=" << node.mesh;
    if (node.camera != kInvalidObjectId) out << " camera=" << node.camera;
    if (node.light != kInvalidObjectId) out << " light=" << node.light;
    WriteTransformFields(out, node.local);
    out << "\n";
  }
  for (const auto& texture : scene.textures) {
    out << "texture id=" << texture.id << " name=" << Quote(texture.name)
        << " uri=" << Quote(texture.uri) << " width=" << texture.width
        << " height=" << texture.height << " color=" << Quote(texture.color_space) << "\n";
  }
  for (const auto& material : scene.materials) {
    out << "material id=" << material.id << " name=" << Quote(material.name) << " base="
        << material.base_color.x << "," << material.base_color.y << "," << material.base_color.z
        << " roughness=" << material.roughness << " metallic=" << material.metallic;
    if (material.base_color_texture != kInvalidObjectId) {
      out << " texture=" << material.base_color_texture;
    }
    out << "\n";
  }
  for (const auto& mesh : scene.meshes) {
    out << "mesh id=" << mesh.id << " name=" << Quote(mesh.name);
    if (mesh.material != kInvalidObjectId) out << " material=" << mesh.material;
    out << "\n";
    for (const auto& vertex : mesh.vertices) {
      out << "vertex mesh=" << mesh.id << " position=" << vertex.position.x << ","
          << vertex.position.y << "," << vertex.position.z << " normal=" << vertex.normal.x
          << "," << vertex.normal.y << "," << vertex.normal.z << " uv=" << vertex.texcoord.x
          << "," << vertex.texcoord.y << "\n";
    }
    for (std::uint32_t index : mesh.indices) {
      out << "index mesh=" << mesh.id << " value=" << index << "\n";
    }
  }
  for (const auto& camera : scene.cameras) {
    out << "camera id=" << camera.id << " name=" << Quote(camera.name)
        << " fov=" << camera.vertical_fov_degrees << " near=" << camera.near_plane
        << " far=" << camera.far_plane << "\n";
  }
  for (const auto& light : scene.lights) {
    out << "light id=" << light.id << " name=" << Quote(light.name)
        << " type=" << LightTypeName(light.type) << " color=" << light.color.x << ","
        << light.color.y << "," << light.color.z << " intensity=" << light.intensity << "\n";
  }
  return out.str();
}

}  // namespace bse
