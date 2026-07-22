#include "bse/parser.hpp"

#include "bse/binary_reader.hpp"
#include "bse/format.hpp"
#include "bse/validator.hpp"

#include <utility>

namespace bse {

namespace {

Result<Vec2> ReadVec2(BinaryReader* reader) {
  auto x = reader->ReadF32();
  if (!x.ok()) {
    return x.status();
  }
  auto y = reader->ReadF32();
  if (!y.ok()) {
    return y.status();
  }
  return Vec2{x.value(), y.value()};
}

Result<Vec3> ReadVec3(BinaryReader* reader) {
  auto x = reader->ReadF32();
  if (!x.ok()) {
    return x.status();
  }
  auto y = reader->ReadF32();
  if (!y.ok()) {
    return y.status();
  }
  auto z = reader->ReadF32();
  if (!z.ok()) {
    return z.status();
  }
  return Vec3{x.value(), y.value(), z.value()};
}

Result<Quat> ReadQuat(BinaryReader* reader) {
  auto x = reader->ReadF32();
  if (!x.ok()) {
    return x.status();
  }
  auto y = reader->ReadF32();
  if (!y.ok()) {
    return y.status();
  }
  auto z = reader->ReadF32();
  if (!z.ok()) {
    return z.status();
  }
  auto w = reader->ReadF32();
  if (!w.ok()) {
    return w.status();
  }
  return Quat{x.value(), y.value(), z.value(), w.value()};
}

Result<Transform> ReadTransform(BinaryReader* reader) {
  auto translation = ReadVec3(reader);
  if (!translation.ok()) {
    return translation.status();
  }
  auto rotation = ReadQuat(reader);
  if (!rotation.ok()) {
    return rotation.status();
  }
  auto scale = ReadVec3(reader);
  if (!scale.ok()) {
    return scale.status();
  }
  return Transform{translation.value(), rotation.value(), scale.value()};
}

Result<std::string> ReadString(BinaryReader* reader) {
  return reader->ReadString(kMaxStringBytes);
}

}  // namespace

Result<Scene> ParseScene(const std::uint8_t* data, std::size_t size) {
  BinaryReader reader(data, size);
  for (std::uint8_t expected : kSceneMagic) {
    auto actual = reader.ReadU8();
    if (!actual.ok()) {
      return actual.status();
    }
    if (actual.value() != expected) {
      return Status::Error(ErrorCode::kInvalidMagic, "input does not start with BSEN magic");
    }
  }

  auto major = reader.ReadU16();
  auto minor = reader.ReadU16();
  auto flags = reader.ReadU32();
  if (!major.ok()) {
    return major.status();
  }
  if (!minor.ok()) {
    return minor.status();
  }
  if (!flags.ok()) {
    return flags.status();
  }
  if (major.value() != kFormatMajor) {
    return Status::Error(ErrorCode::kUnsupportedVersion, "unsupported binary scene major version");
  }

  Scene scene;
  auto name = ReadString(&reader);
  if (!name.ok()) {
    return name.status();
  }
  scene.name = std::move(name.value());

  auto metadata_count = reader.ReadU32();
  if (!metadata_count.ok()) {
    return metadata_count.status();
  }
  for (std::uint32_t i = 0; i < metadata_count.value(); ++i) {
    auto key = ReadString(&reader);
    if (!key.ok()) {
      return key.status();
    }
    auto value = ReadString(&reader);
    if (!value.ok()) {
      return value.status();
    }
    scene.metadata.emplace(std::move(key.value()), std::move(value.value()));
  }

  auto node_count = reader.ReadU32();
  if (!node_count.ok()) {
    return node_count.status();
  }
  for (std::uint32_t i = 0; i < node_count.value(); ++i) {
    Node node;
    auto id = reader.ReadU64();
    auto node_name = ReadString(&reader);
    auto parent = reader.ReadU64();
    auto transform = ReadTransform(&reader);
    auto mesh = reader.ReadU64();
    auto camera = reader.ReadU64();
    auto light = reader.ReadU64();
    auto child_count = reader.ReadU32();
    if (!id.ok()) return id.status();
    if (!node_name.ok()) return node_name.status();
    if (!parent.ok()) return parent.status();
    if (!transform.ok()) return transform.status();
    if (!mesh.ok()) return mesh.status();
    if (!camera.ok()) return camera.status();
    if (!light.ok()) return light.status();
    if (!child_count.ok()) return child_count.status();
    node.id = id.value();
    node.name = std::move(node_name.value());
    node.parent = parent.value();
    node.local = transform.value();
    node.mesh = mesh.value();
    node.camera = camera.value();
    node.light = light.value();
    for (std::uint32_t child_index = 0; child_index < child_count.value(); ++child_index) {
      auto child = reader.ReadU64();
      if (!child.ok()) return child.status();
      node.children.push_back(child.value());
    }
    scene.nodes.push_back(std::move(node));
  }

  auto texture_count = reader.ReadU32();
  if (!texture_count.ok()) return texture_count.status();
  for (std::uint32_t i = 0; i < texture_count.value(); ++i) {
    Texture texture;
    auto id = reader.ReadU64();
    auto texture_name = ReadString(&reader);
    auto uri = ReadString(&reader);
    auto width = reader.ReadU32();
    auto height = reader.ReadU32();
    auto color_space = ReadString(&reader);
    if (!id.ok()) return id.status();
    if (!texture_name.ok()) return texture_name.status();
    if (!uri.ok()) return uri.status();
    if (!width.ok()) return width.status();
    if (!height.ok()) return height.status();
    if (!color_space.ok()) return color_space.status();
    texture.id = id.value();
    texture.name = std::move(texture_name.value());
    texture.uri = std::move(uri.value());
    texture.width = width.value();
    texture.height = height.value();
    texture.color_space = std::move(color_space.value());
    scene.textures.push_back(std::move(texture));
  }

  auto material_count = reader.ReadU32();
  if (!material_count.ok()) return material_count.status();
  for (std::uint32_t i = 0; i < material_count.value(); ++i) {
    Material material;
    auto id = reader.ReadU64();
    auto material_name = ReadString(&reader);
    auto color = ReadVec3(&reader);
    auto roughness = reader.ReadF32();
    auto metallic = reader.ReadF32();
    auto base_color_texture = reader.ReadU64();
    if (!id.ok()) return id.status();
    if (!material_name.ok()) return material_name.status();
    if (!color.ok()) return color.status();
    if (!roughness.ok()) return roughness.status();
    if (!metallic.ok()) return metallic.status();
    if (!base_color_texture.ok()) return base_color_texture.status();
    material.id = id.value();
    material.name = std::move(material_name.value());
    material.base_color = color.value();
    material.roughness = roughness.value();
    material.metallic = metallic.value();
    material.base_color_texture = base_color_texture.value();
    scene.materials.push_back(material);
  }

  auto mesh_count = reader.ReadU32();
  if (!mesh_count.ok()) return mesh_count.status();
  for (std::uint32_t i = 0; i < mesh_count.value(); ++i) {
    Mesh mesh;
    auto id = reader.ReadU64();
    auto mesh_name = ReadString(&reader);
    auto material = reader.ReadU64();
    auto vertex_count = reader.ReadU32();
    if (!id.ok()) return id.status();
    if (!mesh_name.ok()) return mesh_name.status();
    if (!material.ok()) return material.status();
    if (!vertex_count.ok()) return vertex_count.status();
    mesh.id = id.value();
    mesh.name = std::move(mesh_name.value());
    mesh.material = material.value();
    for (std::uint32_t vertex_index = 0; vertex_index < vertex_count.value(); ++vertex_index) {
      auto position = ReadVec3(&reader);
      auto normal = ReadVec3(&reader);
      auto texcoord = ReadVec2(&reader);
      if (!position.ok()) return position.status();
      if (!normal.ok()) return normal.status();
      if (!texcoord.ok()) return texcoord.status();
      mesh.vertices.push_back(Vertex{position.value(), normal.value(), texcoord.value()});
    }
    auto index_count = reader.ReadU32();
    if (!index_count.ok()) return index_count.status();
    for (std::uint32_t index_index = 0; index_index < index_count.value(); ++index_index) {
      auto index = reader.ReadU32();
      if (!index.ok()) return index.status();
      mesh.indices.push_back(index.value());
    }
    scene.meshes.push_back(std::move(mesh));
  }

  auto camera_count = reader.ReadU32();
  if (!camera_count.ok()) return camera_count.status();
  for (std::uint32_t i = 0; i < camera_count.value(); ++i) {
    Camera camera;
    auto id = reader.ReadU64();
    auto camera_name = ReadString(&reader);
    auto fov = reader.ReadF32();
    auto near_plane = reader.ReadF32();
    auto far_plane = reader.ReadF32();
    if (!id.ok()) return id.status();
    if (!camera_name.ok()) return camera_name.status();
    if (!fov.ok()) return fov.status();
    if (!near_plane.ok()) return near_plane.status();
    if (!far_plane.ok()) return far_plane.status();
    camera.id = id.value();
    camera.name = std::move(camera_name.value());
    camera.vertical_fov_degrees = fov.value();
    camera.near_plane = near_plane.value();
    camera.far_plane = far_plane.value();
    scene.cameras.push_back(camera);
  }

  auto light_count = reader.ReadU32();
  if (!light_count.ok()) return light_count.status();
  for (std::uint32_t i = 0; i < light_count.value(); ++i) {
    Light light;
    auto id = reader.ReadU64();
    auto light_name = ReadString(&reader);
    auto type = reader.ReadU8();
    auto color = ReadVec3(&reader);
    auto intensity = reader.ReadF32();
    if (!id.ok()) return id.status();
    if (!light_name.ok()) return light_name.status();
    if (!type.ok()) return type.status();
    if (!color.ok()) return color.status();
    if (!intensity.ok()) return intensity.status();
    light.id = id.value();
    light.name = std::move(light_name.value());
    light.type = static_cast<LightType>(type.value());
    light.color = color.value();
    light.intensity = intensity.value();
    scene.lights.push_back(light);
  }

  auto skeleton_count = reader.ReadU32();
  if (!skeleton_count.ok()) return skeleton_count.status();
  for (std::uint32_t i = 0; i < skeleton_count.value(); ++i) {
    Skeleton skeleton;
    auto id = reader.ReadU64();
    auto skeleton_name = ReadString(&reader);
    auto joint_count = reader.ReadU32();
    if (!id.ok()) return id.status();
    if (!skeleton_name.ok()) return skeleton_name.status();
    if (!joint_count.ok()) return joint_count.status();
    skeleton.id = id.value();
    skeleton.name = std::move(skeleton_name.value());
    for (std::uint32_t joint_index = 0; joint_index < joint_count.value(); ++joint_index) {
      SkeletonJoint joint;
      auto joint_id = reader.ReadU64();
      auto joint_name = ReadString(&reader);
      auto parent = reader.ReadU64();
      auto bind_pose = ReadTransform(&reader);
      if (!joint_id.ok()) return joint_id.status();
      if (!joint_name.ok()) return joint_name.status();
      if (!parent.ok()) return parent.status();
      if (!bind_pose.ok()) return bind_pose.status();
      joint.id = joint_id.value();
      joint.name = std::move(joint_name.value());
      joint.parent = parent.value();
      joint.bind_pose = bind_pose.value();
      skeleton.joints.push_back(joint);
    }
    scene.skeletons.push_back(std::move(skeleton));
  }

  auto animation_count = reader.ReadU32();
  if (!animation_count.ok()) return animation_count.status();
  for (std::uint32_t i = 0; i < animation_count.value(); ++i) {
    Animation animation;
    auto id = reader.ReadU64();
    auto animation_name = ReadString(&reader);
    auto duration = reader.ReadF32();
    auto channel_count = reader.ReadU32();
    if (!id.ok()) return id.status();
    if (!animation_name.ok()) return animation_name.status();
    if (!duration.ok()) return duration.status();
    if (!channel_count.ok()) return channel_count.status();
    animation.id = id.value();
    animation.name = std::move(animation_name.value());
    animation.duration_seconds = duration.value();
    for (std::uint32_t channel_index = 0; channel_index < channel_count.value(); ++channel_index) {
      AnimationChannel channel;
      auto target_node = reader.ReadU64();
      auto key_count = reader.ReadU32();
      if (!target_node.ok()) return target_node.status();
      if (!key_count.ok()) return key_count.status();
      channel.target_node = target_node.value();
      for (std::uint32_t key_index = 0; key_index < key_count.value(); ++key_index) {
        auto time = reader.ReadF32();
        auto transform = ReadTransform(&reader);
        if (!time.ok()) return time.status();
        if (!transform.ok()) return transform.status();
        channel.keys.push_back(AnimationKey{time.value(), transform.value()});
      }
      animation.channels.push_back(std::move(channel));
    }
    scene.animations.push_back(std::move(animation));
  }

  if (!reader.exhausted()) {
    return Status::Error(ErrorCode::kMalformedData, "trailing bytes after scene payload");
  }
  auto validation = ValidateScene(scene);
  if (!validation.ok()) {
    return validation;
  }
  return scene;
}

Result<Scene> ParseScene(const std::vector<std::uint8_t>& data) {
  return ParseScene(data.data(), data.size());
}

}  // namespace bse
