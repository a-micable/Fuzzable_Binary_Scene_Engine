#include "bse/serializer.hpp"

#include "bse/binary_writer.hpp"
#include "bse/format.hpp"
#include "bse/validator.hpp"

#include <algorithm>
#include <vector>

namespace bse {

namespace {

void WriteVec2(BinaryWriter* writer, const Vec2& value) {
  writer->WriteF32(value.x);
  writer->WriteF32(value.y);
}

void WriteVec3(BinaryWriter* writer, const Vec3& value) {
  writer->WriteF32(value.x);
  writer->WriteF32(value.y);
  writer->WriteF32(value.z);
}

void WriteQuat(BinaryWriter* writer, const Quat& value) {
  writer->WriteF32(value.x);
  writer->WriteF32(value.y);
  writer->WriteF32(value.z);
  writer->WriteF32(value.w);
}

void WriteTransform(BinaryWriter* writer, const Transform& value) {
  WriteVec3(writer, value.translation);
  WriteQuat(writer, value.rotation);
  WriteVec3(writer, value.scale);
}

Status WriteString(BinaryWriter* writer, const std::string& value) {
  return writer->WriteString(value);
}

}  // namespace

Result<std::vector<std::uint8_t>> SerializeScene(const Scene& scene) {
  auto validation = ValidateScene(scene);
  if (!validation.ok()) {
    return validation;
  }

  BinaryWriter writer;
  for (std::uint8_t byte : kSceneMagic) {
    writer.WriteU8(byte);
  }
  writer.WriteU16(kFormatMajor);
  writer.WriteU16(kFormatMinor);
  writer.WriteU32(0);

  auto write_string = [&writer](const std::string& value) -> Status {
    return WriteString(&writer, value);
  };

  auto status = write_string(scene.name);
  if (!status.ok()) {
    return status;
  }
  std::vector<std::pair<std::string, std::string>> metadata(scene.metadata.begin(),
                                                            scene.metadata.end());
  std::sort(metadata.begin(), metadata.end(),
            [](const auto& lhs, const auto& rhs) { return lhs.first < rhs.first; });
  writer.WriteU32(static_cast<std::uint32_t>(metadata.size()));
  for (const auto& entry : metadata) {
    status = write_string(entry.first);
    if (!status.ok()) {
      return status;
    }
    status = write_string(entry.second);
    if (!status.ok()) {
      return status;
    }
  }

  writer.WriteU32(static_cast<std::uint32_t>(scene.nodes.size()));
  for (const auto& node : scene.nodes) {
    writer.WriteU64(node.id);
    status = write_string(node.name);
    if (!status.ok()) {
      return status;
    }
    writer.WriteU64(node.parent);
    WriteTransform(&writer, node.local);
    writer.WriteU64(node.mesh);
    writer.WriteU64(node.camera);
    writer.WriteU64(node.light);
    writer.WriteU32(static_cast<std::uint32_t>(node.children.size()));
    for (ObjectId child : node.children) {
      writer.WriteU64(child);
    }
  }

  writer.WriteU32(static_cast<std::uint32_t>(scene.textures.size()));
  for (const auto& texture : scene.textures) {
    writer.WriteU64(texture.id);
    status = write_string(texture.name);
    if (!status.ok()) {
      return status;
    }
    status = write_string(texture.uri);
    if (!status.ok()) {
      return status;
    }
    writer.WriteU32(texture.width);
    writer.WriteU32(texture.height);
    status = write_string(texture.color_space);
    if (!status.ok()) {
      return status;
    }
  }

  writer.WriteU32(static_cast<std::uint32_t>(scene.materials.size()));
  for (const auto& material : scene.materials) {
    writer.WriteU64(material.id);
    status = write_string(material.name);
    if (!status.ok()) {
      return status;
    }
    WriteVec3(&writer, material.base_color);
    writer.WriteF32(material.roughness);
    writer.WriteF32(material.metallic);
    writer.WriteU64(material.base_color_texture);
  }

  writer.WriteU32(static_cast<std::uint32_t>(scene.meshes.size()));
  for (const auto& mesh : scene.meshes) {
    writer.WriteU64(mesh.id);
    status = write_string(mesh.name);
    if (!status.ok()) {
      return status;
    }
    writer.WriteU64(mesh.material);
    writer.WriteU32(static_cast<std::uint32_t>(mesh.vertices.size()));
    for (const auto& vertex : mesh.vertices) {
      WriteVec3(&writer, vertex.position);
      WriteVec3(&writer, vertex.normal);
      WriteVec2(&writer, vertex.texcoord);
    }
    writer.WriteU32(static_cast<std::uint32_t>(mesh.indices.size()));
    for (std::uint32_t index : mesh.indices) {
      writer.WriteU32(index);
    }
  }

  writer.WriteU32(static_cast<std::uint32_t>(scene.cameras.size()));
  for (const auto& camera : scene.cameras) {
    writer.WriteU64(camera.id);
    status = write_string(camera.name);
    if (!status.ok()) {
      return status;
    }
    writer.WriteF32(camera.vertical_fov_degrees);
    writer.WriteF32(camera.near_plane);
    writer.WriteF32(camera.far_plane);
  }

  writer.WriteU32(static_cast<std::uint32_t>(scene.lights.size()));
  for (const auto& light : scene.lights) {
    writer.WriteU64(light.id);
    status = write_string(light.name);
    if (!status.ok()) {
      return status;
    }
    writer.WriteU8(static_cast<std::uint8_t>(light.type));
    WriteVec3(&writer, light.color);
    writer.WriteF32(light.intensity);
  }

  writer.WriteU32(static_cast<std::uint32_t>(scene.skeletons.size()));
  for (const auto& skeleton : scene.skeletons) {
    writer.WriteU64(skeleton.id);
    status = write_string(skeleton.name);
    if (!status.ok()) {
      return status;
    }
    writer.WriteU32(static_cast<std::uint32_t>(skeleton.joints.size()));
    for (const auto& joint : skeleton.joints) {
      writer.WriteU64(joint.id);
      status = write_string(joint.name);
      if (!status.ok()) {
        return status;
      }
      writer.WriteU64(joint.parent);
      WriteTransform(&writer, joint.bind_pose);
    }
  }

  writer.WriteU32(static_cast<std::uint32_t>(scene.animations.size()));
  for (const auto& animation : scene.animations) {
    writer.WriteU64(animation.id);
    status = write_string(animation.name);
    if (!status.ok()) {
      return status;
    }
    writer.WriteF32(animation.duration_seconds);
    writer.WriteU32(static_cast<std::uint32_t>(animation.channels.size()));
    for (const auto& channel : animation.channels) {
      writer.WriteU64(channel.target_node);
      writer.WriteU32(static_cast<std::uint32_t>(channel.keys.size()));
      for (const auto& key : channel.keys) {
        writer.WriteF32(key.time_seconds);
        WriteTransform(&writer, key.transform);
      }
    }
  }

  return writer.TakeBytes();
}

}  // namespace bse
