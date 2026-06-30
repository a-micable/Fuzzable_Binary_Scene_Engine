#pragma once

#include "bse/scene.hpp"
#include "bse/status.hpp"

#include <string>
#include <vector>

namespace bse {

struct ObjWriteOptions {
  bool include_normals = true;
  bool include_texcoords = true;
  bool include_object_names = true;
};

struct ObjReadOptions {
  ObjectId mesh_id = 1;
  ObjectId material_id = kInvalidObjectId;
  std::string mesh_name = "obj_mesh";
};

Result<std::string> WriteObjMesh(const Mesh& mesh, const ObjWriteOptions& options = ObjWriteOptions{});
Result<std::string> WriteObjScene(const Scene& scene, const ObjWriteOptions& options = ObjWriteOptions{});
Result<Mesh> ReadObjMesh(const std::string& text, const ObjReadOptions& options = ObjReadOptions{});
std::vector<std::string> SplitObjLines(const std::string& text);

}  // namespace bse
