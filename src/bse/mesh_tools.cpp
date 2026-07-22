#include "bse/mesh_tools.hpp"

#include "bse/scene_stats.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <sstream>
#include <unordered_map>

namespace bse {

namespace {

constexpr float kEpsilon = 0.000001F;

std::uint32_t PushVertex(std::vector<Vertex>* vertices, Vertex vertex) {
  vertices->push_back(vertex);
  return static_cast<std::uint32_t>(vertices->size() - 1U);
}

void AddTriangle(Mesh* mesh, std::uint32_t a, std::uint32_t b, std::uint32_t c) {
  mesh->indices.push_back(a);
  mesh->indices.push_back(b);
  mesh->indices.push_back(c);
}

Vertex MakeVertex(Vec3 position, Vec3 normal, Vec2 uv) {
  Vertex vertex;
  vertex.position = position;
  vertex.normal = normal;
  vertex.texcoord = uv;
  return vertex;
}

int Quantize(float value, float tolerance) {
  if (tolerance <= 0.0F) {
    tolerance = 0.0001F;
  }
  return static_cast<int>(std::lround(value / tolerance));
}

VertexKey MakeKey(const Vertex& vertex, float tolerance) {
  return {Quantize(vertex.position.x, tolerance), Quantize(vertex.position.y, tolerance),
          Quantize(vertex.position.z, tolerance), Quantize(vertex.normal.x, tolerance),
          Quantize(vertex.normal.y, tolerance), Quantize(vertex.normal.z, tolerance),
          Quantize(vertex.texcoord.x, tolerance), Quantize(vertex.texcoord.y, tolerance)};
}

struct VertexKeyHash {
  std::size_t operator()(const VertexKey& key) const {
    std::size_t hash = 1469598103934665603ULL;
    auto mix = [&hash](int value) {
      hash ^= static_cast<std::size_t>(value);
      hash *= 1099511628211ULL;
    };
    mix(key.x);
    mix(key.y);
    mix(key.z);
    mix(key.nx);
    mix(key.ny);
    mix(key.nz);
    mix(key.u);
    mix(key.v);
    return hash;
  }
};

bool IsValidIndex(const Mesh& mesh, std::uint32_t index) {
  return index < mesh.vertices.size();
}

bool IsDegenerate(const Mesh& mesh, std::uint32_t a, std::uint32_t b, std::uint32_t c) {
  if (!IsValidIndex(mesh, a) || !IsValidIndex(mesh, b) || !IsValidIndex(mesh, c)) {
    return true;
  }
  return TriangleArea(mesh.vertices[a], mesh.vertices[b], mesh.vertices[c]) < kEpsilon;
}

}  // namespace

Vec3 Add(Vec3 lhs, Vec3 rhs) { return {lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z}; }

Vec3 Subtract(Vec3 lhs, Vec3 rhs) { return {lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z}; }

Vec3 Multiply(Vec3 lhs, float scale) { return {lhs.x * scale, lhs.y * scale, lhs.z * scale}; }

float Dot(Vec3 lhs, Vec3 rhs) { return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z; }

Vec3 Cross(Vec3 lhs, Vec3 rhs) {
  return {lhs.y * rhs.z - lhs.z * rhs.y, lhs.z * rhs.x - lhs.x * rhs.z,
          lhs.x * rhs.y - lhs.y * rhs.x};
}

float Length(Vec3 value) { return std::sqrt(Dot(value, value)); }

Vec3 Normalize(Vec3 value) {
  const float length = Length(value);
  if (length < kEpsilon) {
    return {};
  }
  return Multiply(value, 1.0F / length);
}

float TriangleArea(const Vertex& a, const Vertex& b, const Vertex& c) {
  const Vec3 ab = Subtract(b.position, a.position);
  const Vec3 ac = Subtract(c.position, a.position);
  return Length(Cross(ab, ac)) * 0.5F;
}

Bounds3 ComputeVertexBounds(const std::vector<Vertex>& vertices) {
  Mesh mesh;
  mesh.vertices = vertices;
  return ComputeMeshBounds(mesh);
}

Mesh MakePlaneMesh(float width, float depth, std::uint32_t columns, std::uint32_t rows,
                   const MeshBuildOptions& options) {
  Mesh mesh;
  mesh.id = options.id;
  mesh.name = options.name;
  mesh.material = options.material;
  columns = std::max<std::uint32_t>(columns, 1);
  rows = std::max<std::uint32_t>(rows, 1);
  const float half_width = width * 0.5F;
  const float half_depth = depth * 0.5F;
  for (std::uint32_t y = 0; y <= rows; ++y) {
    for (std::uint32_t x = 0; x <= columns; ++x) {
      const float u = static_cast<float>(x) / static_cast<float>(columns);
      const float v = static_cast<float>(y) / static_cast<float>(rows);
      const float px = -half_width + width * u;
      const float pz = -half_depth + depth * v;
      mesh.vertices.push_back(MakeVertex({px, 0.0F, pz}, {0.0F, 1.0F, 0.0F}, {u, v}));
    }
  }
  const std::uint32_t stride = columns + 1U;
  for (std::uint32_t y = 0; y < rows; ++y) {
    for (std::uint32_t x = 0; x < columns; ++x) {
      const std::uint32_t a = y * stride + x;
      const std::uint32_t b = a + 1U;
      const std::uint32_t c = a + stride;
      const std::uint32_t d = c + 1U;
      AddTriangle(&mesh, a, c, b);
      AddTriangle(&mesh, b, c, d);
    }
  }
  return mesh;
}

Mesh MakeBoxMesh(float width, float height, float depth, const MeshBuildOptions& options) {
  Mesh mesh;
  mesh.id = options.id;
  mesh.name = options.name;
  mesh.material = options.material;
  const float x = width * 0.5F;
  const float y = height * 0.5F;
  const float z = depth * 0.5F;
  struct Face {
    Vec3 normal;
    Vec3 a;
    Vec3 b;
    Vec3 c;
    Vec3 d;
  };
  const std::vector<Face> faces = {
      {{0.0F, 0.0F, 1.0F}, {-x, -y, z}, {x, -y, z}, {-x, y, z}, {x, y, z}},
      {{0.0F, 0.0F, -1.0F}, {x, -y, -z}, {-x, -y, -z}, {x, y, -z}, {-x, y, -z}},
      {{1.0F, 0.0F, 0.0F}, {x, -y, z}, {x, -y, -z}, {x, y, z}, {x, y, -z}},
      {{-1.0F, 0.0F, 0.0F}, {-x, -y, -z}, {-x, -y, z}, {-x, y, -z}, {-x, y, z}},
      {{0.0F, 1.0F, 0.0F}, {-x, y, z}, {x, y, z}, {-x, y, -z}, {x, y, -z}},
      {{0.0F, -1.0F, 0.0F}, {-x, -y, -z}, {x, -y, -z}, {-x, -y, z}, {x, -y, z}},
  };
  for (const auto& face : faces) {
    const std::uint32_t a = PushVertex(&mesh.vertices, MakeVertex(face.a, face.normal, {0.0F, 0.0F}));
    const std::uint32_t b = PushVertex(&mesh.vertices, MakeVertex(face.b, face.normal, {1.0F, 0.0F}));
    const std::uint32_t c = PushVertex(&mesh.vertices, MakeVertex(face.c, face.normal, {0.0F, 1.0F}));
    const std::uint32_t d = PushVertex(&mesh.vertices, MakeVertex(face.d, face.normal, {1.0F, 1.0F}));
    AddTriangle(&mesh, a, c, b);
    AddTriangle(&mesh, b, c, d);
  }
  return mesh;
}

Mesh MakeUvSphereMesh(float radius, std::uint32_t slices, std::uint32_t stacks,
                      const MeshBuildOptions& options) {
  Mesh mesh;
  mesh.id = options.id;
  mesh.name = options.name;
  mesh.material = options.material;
  slices = std::max<std::uint32_t>(slices, 3);
  stacks = std::max<std::uint32_t>(stacks, 2);
  constexpr float pi = 3.14159265358979323846F;
  for (std::uint32_t stack = 0; stack <= stacks; ++stack) {
    const float v = static_cast<float>(stack) / static_cast<float>(stacks);
    const float phi = v * pi;
    const float y = std::cos(phi);
    const float ring = std::sin(phi);
    for (std::uint32_t slice = 0; slice <= slices; ++slice) {
      const float u = static_cast<float>(slice) / static_cast<float>(slices);
      const float theta = u * pi * 2.0F;
      Vec3 normal{std::cos(theta) * ring, y, std::sin(theta) * ring};
      mesh.vertices.push_back(MakeVertex(Multiply(normal, radius), normal, {u, v}));
    }
  }
  const std::uint32_t stride = slices + 1U;
  for (std::uint32_t stack = 0; stack < stacks; ++stack) {
    for (std::uint32_t slice = 0; slice < slices; ++slice) {
      const std::uint32_t a = stack * stride + slice;
      const std::uint32_t b = a + 1U;
      const std::uint32_t c = a + stride;
      const std::uint32_t d = c + 1U;
      if (stack != 0) {
        AddTriangle(&mesh, a, c, b);
      }
      if (stack + 1U != stacks) {
        AddTriangle(&mesh, b, c, d);
      }
    }
  }
  return mesh;
}

Mesh MakeCylinderMesh(float radius, float height, std::uint32_t segments,
                      const MeshBuildOptions& options) {
  Mesh mesh;
  mesh.id = options.id;
  mesh.name = options.name;
  mesh.material = options.material;
  segments = std::max<std::uint32_t>(segments, 3);
  constexpr float pi = 3.14159265358979323846F;
  const float half_height = height * 0.5F;
  for (std::uint32_t i = 0; i <= segments; ++i) {
    const float u = static_cast<float>(i) / static_cast<float>(segments);
    const float theta = u * pi * 2.0F;
    Vec3 normal{std::cos(theta), 0.0F, std::sin(theta)};
    Vec3 bottom{normal.x * radius, -half_height, normal.z * radius};
    Vec3 top{normal.x * radius, half_height, normal.z * radius};
    mesh.vertices.push_back(MakeVertex(bottom, normal, {u, 0.0F}));
    mesh.vertices.push_back(MakeVertex(top, normal, {u, 1.0F}));
  }
  for (std::uint32_t i = 0; i < segments; ++i) {
    const std::uint32_t a = i * 2U;
    const std::uint32_t b = a + 1U;
    const std::uint32_t c = a + 2U;
    const std::uint32_t d = a + 3U;
    AddTriangle(&mesh, a, b, c);
    AddTriangle(&mesh, c, b, d);
  }
  const std::uint32_t bottom_center =
      PushVertex(&mesh.vertices, MakeVertex({0.0F, -half_height, 0.0F}, {0.0F, -1.0F, 0.0F}, {0.5F, 0.5F}));
  const std::uint32_t top_center =
      PushVertex(&mesh.vertices, MakeVertex({0.0F, half_height, 0.0F}, {0.0F, 1.0F, 0.0F}, {0.5F, 0.5F}));
  for (std::uint32_t i = 0; i < segments; ++i) {
    const std::uint32_t a = i * 2U;
    const std::uint32_t b = ((i + 1U) % segments) * 2U;
    AddTriangle(&mesh, bottom_center, b, a);
    AddTriangle(&mesh, top_center, a + 1U, b + 1U);
  }
  return mesh;
}

void RecalculateNormals(Mesh* mesh) {
  for (auto& vertex : mesh->vertices) {
    vertex.normal = {};
  }
  for (std::size_t i = 0; i + 2U < mesh->indices.size(); i += 3U) {
    const auto a = mesh->indices[i];
    const auto b = mesh->indices[i + 1U];
    const auto c = mesh->indices[i + 2U];
    if (!IsValidIndex(*mesh, a) || !IsValidIndex(*mesh, b) || !IsValidIndex(*mesh, c)) {
      continue;
    }
    const Vec3 normal = Cross(Subtract(mesh->vertices[b].position, mesh->vertices[a].position),
                              Subtract(mesh->vertices[c].position, mesh->vertices[a].position));
    mesh->vertices[a].normal = Add(mesh->vertices[a].normal, normal);
    mesh->vertices[b].normal = Add(mesh->vertices[b].normal, normal);
    mesh->vertices[c].normal = Add(mesh->vertices[c].normal, normal);
  }
  for (auto& vertex : mesh->vertices) {
    vertex.normal = Normalize(vertex.normal);
  }
}

MeshRepairReport RepairMesh(Mesh* mesh) {
  MeshRepairReport report;
  std::vector<std::uint32_t> repaired;
  repaired.reserve(mesh->indices.size());
  for (std::size_t i = 0; i + 2U < mesh->indices.size(); i += 3U) {
    const auto a = mesh->indices[i];
    const auto b = mesh->indices[i + 1U];
    const auto c = mesh->indices[i + 2U];
    if (!IsValidIndex(*mesh, a) || !IsValidIndex(*mesh, b) || !IsValidIndex(*mesh, c)) {
      ++report.removed_out_of_range_indices;
      continue;
    }
    if (IsDegenerate(*mesh, a, b, c)) {
      ++report.removed_degenerate_triangles;
      continue;
    }
    repaired.push_back(a);
    repaired.push_back(b);
    repaired.push_back(c);
  }
  mesh->indices = std::move(repaired);
  RecalculateNormals(mesh);
  report.generated_normals = mesh->vertices.size();
  return report;
}

MeshRepairReport WeldVertices(Mesh* mesh, float tolerance) {
  MeshRepairReport report;
  std::unordered_map<VertexKey, std::uint32_t, VertexKeyHash> remap;
  std::vector<Vertex> vertices;
  vertices.reserve(mesh->vertices.size());
  std::vector<std::uint32_t> old_to_new(mesh->vertices.size(), 0);
  for (std::size_t i = 0; i < mesh->vertices.size(); ++i) {
    const auto key = MakeKey(mesh->vertices[i], tolerance);
    auto it = remap.find(key);
    if (it == remap.end()) {
      const auto new_index = static_cast<std::uint32_t>(vertices.size());
      remap.emplace(key, new_index);
      old_to_new[i] = new_index;
      vertices.push_back(mesh->vertices[i]);
    } else {
      old_to_new[i] = it->second;
      ++report.welded_vertices;
    }
  }
  for (auto& index : mesh->indices) {
    if (index < old_to_new.size()) {
      index = old_to_new[index];
    }
  }
  mesh->vertices = std::move(vertices);
  return report;
}

Result<Mesh> CombineMeshes(ObjectId id, std::string name, const std::vector<Mesh>& meshes) {
  Mesh combined;
  combined.id = id;
  combined.name = std::move(name);
  std::size_t vertex_offset = 0;
  for (const auto& mesh : meshes) {
    if (vertex_offset + mesh.vertices.size() > std::numeric_limits<std::uint32_t>::max()) {
      return Status::Error(ErrorCode::kLimitExceeded, "combined mesh exceeds u32 index range");
    }
    combined.vertices.insert(combined.vertices.end(), mesh.vertices.begin(), mesh.vertices.end());
    for (auto index : mesh.indices) {
      combined.indices.push_back(static_cast<std::uint32_t>(vertex_offset + index));
    }
    if (combined.material == kInvalidObjectId) {
      combined.material = mesh.material;
    }
    vertex_offset += mesh.vertices.size();
  }
  auto status = ValidateMeshTopology(combined);
  if (!status.ok()) {
    return status.status();
  }
  return combined;
}

Result<void*> ValidateMeshTopology(const Mesh& mesh) {
  if (!mesh.indices.empty() && (mesh.indices.size() % 3U) != 0U) {
    return Status::Error(ErrorCode::kMalformedData, "mesh index count is not divisible by three");
  }
  for (std::uint32_t index : mesh.indices) {
    if (index >= mesh.vertices.size()) {
      return Status::Error(ErrorCode::kMalformedData, "mesh index is outside vertex range");
    }
  }
  return static_cast<void*>(nullptr);
}

std::string SummarizeMesh(const Mesh& mesh) {
  std::ostringstream out;
  out << "mesh " << mesh.id << " " << mesh.name << "\n";
  out << "vertices=" << mesh.vertices.size() << "\n";
  out << "indices=" << mesh.indices.size() << "\n";
  out << "triangles=" << (mesh.indices.empty() ? mesh.vertices.size() / 3U : mesh.indices.size() / 3U)
      << "\n";
  auto bounds = ComputeMeshBounds(mesh);
  if (bounds.valid) {
    out << "bounds_min=" << bounds.min.x << "," << bounds.min.y << "," << bounds.min.z << "\n";
    out << "bounds_max=" << bounds.max.x << "," << bounds.max.y << "," << bounds.max.z << "\n";
  }
  return out.str();
}

}  // namespace bse
