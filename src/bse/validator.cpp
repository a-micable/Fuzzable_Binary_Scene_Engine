#include "bse/validator.hpp"

#include <cmath>
#include <unordered_set>

namespace bse {

namespace {

template <typename T>
bool InsertUnique(ObjectId id, const char* type, std::unordered_set<ObjectId>* ids,
                  DiagnosticSink* diagnostics) {
  static_cast<void>(sizeof(T));
  if (id == kInvalidObjectId) {
    if (diagnostics != nullptr) {
      diagnostics->Error("invalid_id", std::string(type) + " has reserved object id 0");
    }
    return false;
  }
  if (!ids->insert(id).second) {
    if (diagnostics != nullptr) {
      diagnostics->Error("duplicate_id", std::string(type) + " id is duplicated");
    }
    return false;
  }
  return true;
}

bool Contains(const std::unordered_set<ObjectId>& ids, ObjectId id) {
  return id == kInvalidObjectId || ids.find(id) != ids.end();
}

bool IsValidLightType(LightType type) {
  return type == LightType::kDirectional || type == LightType::kPoint || type == LightType::kSpot;
}

bool IsFinitePositive(float value) {
  return std::isfinite(value) && value > 0.0F;
}

void AddError(DiagnosticSink* diagnostics, const std::string& code, const std::string& message) {
  if (diagnostics != nullptr) {
    diagnostics->Error(code, message);
  }
}

}  // namespace

Status ValidateScene(const Scene& scene, DiagnosticSink* diagnostics, const ValidationLimits& limits) {
  bool ok = true;
  std::unordered_set<ObjectId> node_ids;
  std::unordered_set<ObjectId> mesh_ids;
  std::unordered_set<ObjectId> material_ids;
  std::unordered_set<ObjectId> texture_ids;
  std::unordered_set<ObjectId> camera_ids;
  std::unordered_set<ObjectId> light_ids;
  std::size_t vertex_count = 0;
  std::size_t index_count = 0;

  if (scene.nodes.size() > limits.max_nodes) {
    ok = false;
    if (diagnostics != nullptr) {
      diagnostics->Error("node_limit", "node count exceeds validation limit");
    }
  }

  for (const auto& node : scene.nodes) {
    ok = InsertUnique<Node>(node.id, "node", &node_ids, diagnostics) && ok;
  }
  for (const auto& mesh : scene.meshes) {
    ok = InsertUnique<Mesh>(mesh.id, "mesh", &mesh_ids, diagnostics) && ok;
    vertex_count += mesh.vertices.size();
    index_count += mesh.indices.size();
  }
  for (const auto& material : scene.materials) {
    ok = InsertUnique<Material>(material.id, "material", &material_ids, diagnostics) && ok;
  }
  for (const auto& texture : scene.textures) {
    ok = InsertUnique<Texture>(texture.id, "texture", &texture_ids, diagnostics) && ok;
  }
  for (const auto& camera : scene.cameras) {
    ok = InsertUnique<Camera>(camera.id, "camera", &camera_ids, diagnostics) && ok;
  }
  for (const auto& light : scene.lights) {
    ok = InsertUnique<Light>(light.id, "light", &light_ids, diagnostics) && ok;
  }

  if (vertex_count > limits.max_vertices || index_count > limits.max_indices) {
    ok = false;
    if (diagnostics != nullptr) {
      diagnostics->Error("geometry_limit", "geometry exceeds validation limits");
    }
  }

  for (const auto& node : scene.nodes) {
    if (!Contains(node_ids, node.parent) || !Contains(mesh_ids, node.mesh) ||
        !Contains(camera_ids, node.camera) || !Contains(light_ids, node.light)) {
      ok = false;
      AddError(diagnostics, "missing_reference", "node references an unknown object");
    }
    for (ObjectId child : node.children) {
      if (!Contains(node_ids, child)) {
        ok = false;
        AddError(diagnostics, "missing_reference", "node child list references an unknown node");
      }
    }
  }
  for (const auto& mesh : scene.meshes) {
    if (!Contains(material_ids, mesh.material)) {
      ok = false;
      AddError(diagnostics, "missing_reference", "mesh references an unknown material");
    }
    for (std::uint32_t index : mesh.indices) {
      if (index >= mesh.vertices.size()) {
        ok = false;
        AddError(diagnostics, "index_out_of_range", "mesh index references a missing vertex");
        break;
      }
    }
  }
  for (const auto& material : scene.materials) {
    if (!Contains(texture_ids, material.base_color_texture)) {
      ok = false;
      AddError(diagnostics, "missing_reference", "material references an unknown texture");
    }
    if (material.roughness < 0.0F || material.roughness > 1.0F || material.metallic < 0.0F ||
        material.metallic > 1.0F) {
      ok = false;
      AddError(diagnostics, "material_range", "material roughness and metallic must be in [0, 1]");
    }
  }
  for (const auto& texture : scene.textures) {
    if (texture.width == 0 || texture.height == 0) {
      ok = false;
      AddError(diagnostics, "texture_size", "texture dimensions must be non-zero");
    }
  }
  for (const auto& camera : scene.cameras) {
    if (!IsFinitePositive(camera.vertical_fov_degrees) || camera.vertical_fov_degrees >= 180.0F ||
        !IsFinitePositive(camera.near_plane) || !IsFinitePositive(camera.far_plane) ||
        camera.near_plane >= camera.far_plane) {
      ok = false;
      AddError(diagnostics, "camera_range", "camera FOV and clipping planes are invalid");
    }
  }
  for (const auto& light : scene.lights) {
    if (!IsValidLightType(light.type) || !IsFinitePositive(light.intensity)) {
      ok = false;
      AddError(diagnostics, "light_range", "light type or intensity is invalid");
    }
  }
  for (const auto& skeleton : scene.skeletons) {
    std::unordered_set<ObjectId> joint_ids;
    for (const auto& joint : skeleton.joints) {
      ok = InsertUnique<SkeletonJoint>(joint.id, "skeleton joint", &joint_ids, diagnostics) && ok;
    }
    for (const auto& joint : skeleton.joints) {
      if (!Contains(joint_ids, joint.parent)) {
        ok = false;
        AddError(diagnostics, "missing_reference", "skeleton joint references an unknown parent");
      }
    }
  }
  for (const auto& animation : scene.animations) {
    if (animation.duration_seconds < 0.0F || !std::isfinite(animation.duration_seconds)) {
      ok = false;
      AddError(diagnostics, "animation_range", "animation duration is invalid");
    }
    for (const auto& channel : animation.channels) {
      if (!Contains(node_ids, channel.target_node)) {
        ok = false;
        AddError(diagnostics, "missing_reference", "animation channel references an unknown node");
      }
      float previous_time = -1.0F;
      for (const auto& key : channel.keys) {
        if (key.time_seconds < 0.0F || key.time_seconds > animation.duration_seconds ||
            !std::isfinite(key.time_seconds) || key.time_seconds < previous_time) {
          ok = false;
          AddError(diagnostics, "animation_key_range", "animation keys must be ordered in range");
          break;
        }
        previous_time = key.time_seconds;
      }
    }
  }

  if (!ok) {
    return Status::Error(ErrorCode::kValidationFailed, "scene validation failed");
  }
  return Status::Ok();
}

}  // namespace bse
