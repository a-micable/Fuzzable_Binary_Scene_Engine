#pragma once

#include "bse/scene.hpp"
#include "bse/scene_stats.hpp"
#include "bse/status.hpp"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace bse {

struct MeshRepairReport {
  std::size_t removed_degenerate_triangles = 0;
  std::size_t removed_out_of_range_indices = 0;
  std::size_t welded_vertices = 0;
  std::size_t generated_normals = 0;
};

struct MeshBuildOptions {
  ObjectId id = 1;
  std::string name = "mesh";
  ObjectId material = kInvalidObjectId;
};

struct VertexKey {
  int x = 0;
  int y = 0;
  int z = 0;
  int nx = 0;
  int ny = 0;
  int nz = 0;
  int u = 0;
  int v = 0;
};

inline bool operator==(const VertexKey& lhs, const VertexKey& rhs) {
  return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z && lhs.nx == rhs.nx &&
         lhs.ny == rhs.ny && lhs.nz == rhs.nz && lhs.u == rhs.u && lhs.v == rhs.v;
}

Vec3 Add(Vec3 lhs, Vec3 rhs);
Vec3 Subtract(Vec3 lhs, Vec3 rhs);
Vec3 Multiply(Vec3 lhs, float scale);
float Dot(Vec3 lhs, Vec3 rhs);
Vec3 Cross(Vec3 lhs, Vec3 rhs);
float Length(Vec3 value);
Vec3 Normalize(Vec3 value);
float TriangleArea(const Vertex& a, const Vertex& b, const Vertex& c);
Bounds3 ComputeVertexBounds(const std::vector<Vertex>& vertices);
Mesh MakePlaneMesh(float width, float depth, std::uint32_t columns, std::uint32_t rows,
                   const MeshBuildOptions& options = MeshBuildOptions{});
Mesh MakeBoxMesh(float width, float height, float depth,
                 const MeshBuildOptions& options = MeshBuildOptions{});
Mesh MakeUvSphereMesh(float radius, std::uint32_t slices, std::uint32_t stacks,
                      const MeshBuildOptions& options = MeshBuildOptions{});
Mesh MakeCylinderMesh(float radius, float height, std::uint32_t segments,
                      const MeshBuildOptions& options = MeshBuildOptions{});
void RecalculateNormals(Mesh* mesh);
MeshRepairReport RepairMesh(Mesh* mesh);
MeshRepairReport WeldVertices(Mesh* mesh, float tolerance);
Result<Mesh> CombineMeshes(ObjectId id, std::string name, const std::vector<Mesh>& meshes);
Result<void*> ValidateMeshTopology(const Mesh& mesh);
std::string SummarizeMesh(const Mesh& mesh);

}  // namespace bse
