#include "bse/binary_inspector.hpp"

#include "bse/binary_reader.hpp"
#include "bse/format.hpp"

#include <sstream>

namespace bse {

namespace {

struct InspectReader {
  explicit InspectReader(const std::vector<std::uint8_t>& bytes) : reader(bytes) {}
  BinaryReader reader;
  BinaryInspection inspection;
};

void AddField(BinaryInspection* inspection, BinaryFieldKind kind, std::string name,
              std::size_t offset, std::size_t size, std::string value) {
  inspection->fields.push_back({kind, std::move(name), offset, size, std::move(value)});
}

Result<std::uint8_t> ReadU8(InspectReader* in, const std::string& name, BinaryFieldKind kind) {
  const auto offset = in->reader.position();
  auto value = in->reader.ReadU8();
  if (!value.ok()) return value.status();
  AddField(&in->inspection, kind, name, offset, 1, std::to_string(value.value()));
  return value.value();
}

Result<std::uint16_t> ReadU16(InspectReader* in, const std::string& name, BinaryFieldKind kind) {
  const auto offset = in->reader.position();
  auto value = in->reader.ReadU16();
  if (!value.ok()) return value.status();
  AddField(&in->inspection, kind, name, offset, 2, std::to_string(value.value()));
  return value.value();
}

Result<std::uint32_t> ReadU32(InspectReader* in, const std::string& name, BinaryFieldKind kind) {
  const auto offset = in->reader.position();
  auto value = in->reader.ReadU32();
  if (!value.ok()) return value.status();
  AddField(&in->inspection, kind, name, offset, 4, std::to_string(value.value()));
  return value.value();
}

Result<std::uint64_t> ReadU64(InspectReader* in, const std::string& name) {
  const auto offset = in->reader.position();
  auto value = in->reader.ReadU64();
  if (!value.ok()) return value.status();
  AddField(&in->inspection, BinaryFieldKind::kObjectId, name, offset, 8, std::to_string(value.value()));
  return value.value();
}

Result<float> ReadF32(InspectReader* in, const std::string& name) {
  const auto offset = in->reader.position();
  auto value = in->reader.ReadF32();
  if (!value.ok()) return value.status();
  std::ostringstream out;
  out << value.value();
  AddField(&in->inspection, BinaryFieldKind::kFloat, name, offset, 4, out.str());
  return value.value();
}

Result<std::string> ReadString(InspectReader* in, const std::string& name) {
  const auto offset = in->reader.position();
  auto value = in->reader.ReadString(kMaxStringBytes);
  if (!value.ok()) return value.status();
  AddField(&in->inspection, BinaryFieldKind::kString, name, offset, in->reader.position() - offset,
           value.value());
  return value.value();
}

Status SkipTransform(InspectReader* in, const std::string& prefix) {
  for (const auto& field : {"translation.x", "translation.y", "translation.z", "rotation.x",
                            "rotation.y", "rotation.z", "rotation.w", "scale.x", "scale.y",
                            "scale.z"}) {
    auto value = ReadF32(in, prefix + "." + field);
    if (!value.ok()) return value.status();
  }
  return Status::Ok();
}

Status InspectNodes(InspectReader* in) {
  auto count = ReadU32(in, "nodes.count", BinaryFieldKind::kCount);
  if (!count.ok()) return count.status();
  for (std::uint32_t i = 0; i < count.value(); ++i) {
    const auto prefix = "nodes[" + std::to_string(i) + "]";
    auto id = ReadU64(in, prefix + ".id");
    if (!id.ok()) return id.status();
    auto name = ReadString(in, prefix + ".name");
    if (!name.ok()) return name.status();
    auto parent = ReadU64(in, prefix + ".parent");
    if (!parent.ok()) return parent.status();
    auto transform = SkipTransform(in, prefix + ".local");
    if (!transform.ok()) return transform;
    auto mesh = ReadU64(in, prefix + ".mesh");
    auto camera = ReadU64(in, prefix + ".camera");
    auto light = ReadU64(in, prefix + ".light");
    auto children = ReadU32(in, prefix + ".children.count", BinaryFieldKind::kCount);
    if (!mesh.ok()) return mesh.status();
    if (!camera.ok()) return camera.status();
    if (!light.ok()) return light.status();
    if (!children.ok()) return children.status();
    for (std::uint32_t child = 0; child < children.value(); ++child) {
      auto id = ReadU64(in, prefix + ".children[" + std::to_string(child) + "]");
      if (!id.ok()) return id.status();
    }
  }
  return Status::Ok();
}

Status InspectTextures(InspectReader* in) {
  auto count = ReadU32(in, "textures.count", BinaryFieldKind::kCount);
  if (!count.ok()) return count.status();
  for (std::uint32_t i = 0; i < count.value(); ++i) {
    const auto prefix = "textures[" + std::to_string(i) + "]";
    auto id = ReadU64(in, prefix + ".id");
    auto name = ReadString(in, prefix + ".name");
    auto uri = ReadString(in, prefix + ".uri");
    auto width = ReadU32(in, prefix + ".width", BinaryFieldKind::kPayload);
    auto height = ReadU32(in, prefix + ".height", BinaryFieldKind::kPayload);
    auto color = ReadString(in, prefix + ".color_space");
    if (!id.ok()) return id.status();
    if (!name.ok()) return name.status();
    if (!uri.ok()) return uri.status();
    if (!width.ok()) return width.status();
    if (!height.ok()) return height.status();
    if (!color.ok()) return color.status();
  }
  return Status::Ok();
}

Status InspectMaterials(InspectReader* in) {
  auto count = ReadU32(in, "materials.count", BinaryFieldKind::kCount);
  if (!count.ok()) return count.status();
  for (std::uint32_t i = 0; i < count.value(); ++i) {
    const auto prefix = "materials[" + std::to_string(i) + "]";
    auto id = ReadU64(in, prefix + ".id");
    auto name = ReadString(in, prefix + ".name");
    if (!id.ok()) return id.status();
    if (!name.ok()) return name.status();
    for (const auto& field : {"base.r", "base.g", "base.b", "roughness", "metallic"}) {
      auto value = ReadF32(in, prefix + "." + field);
      if (!value.ok()) return value.status();
    }
    auto texture = ReadU64(in, prefix + ".base_color_texture");
    if (!texture.ok()) return texture.status();
  }
  return Status::Ok();
}

Status InspectMeshes(InspectReader* in) {
  auto count = ReadU32(in, "meshes.count", BinaryFieldKind::kCount);
  if (!count.ok()) return count.status();
  for (std::uint32_t i = 0; i < count.value(); ++i) {
    const auto prefix = "meshes[" + std::to_string(i) + "]";
    auto id = ReadU64(in, prefix + ".id");
    auto name = ReadString(in, prefix + ".name");
    auto material = ReadU64(in, prefix + ".material");
    auto vertices = ReadU32(in, prefix + ".vertices.count", BinaryFieldKind::kCount);
    if (!id.ok()) return id.status();
    if (!name.ok()) return name.status();
    if (!material.ok()) return material.status();
    if (!vertices.ok()) return vertices.status();
    for (std::uint32_t v = 0; v < vertices.value(); ++v) {
      for (const auto& field : {"position.x", "position.y", "position.z", "normal.x", "normal.y",
                                "normal.z", "uv.x", "uv.y"}) {
        auto value = ReadF32(in, prefix + ".vertices[" + std::to_string(v) + "]." + field);
        if (!value.ok()) return value.status();
      }
    }
    auto indices = ReadU32(in, prefix + ".indices.count", BinaryFieldKind::kCount);
    if (!indices.ok()) return indices.status();
    for (std::uint32_t idx = 0; idx < indices.value(); ++idx) {
      auto value = ReadU32(in, prefix + ".indices[" + std::to_string(idx) + "]",
                           BinaryFieldKind::kPayload);
      if (!value.ok()) return value.status();
    }
  }
  return Status::Ok();
}

Status InspectCameras(InspectReader* in) {
  auto count = ReadU32(in, "cameras.count", BinaryFieldKind::kCount);
  if (!count.ok()) return count.status();
  for (std::uint32_t i = 0; i < count.value(); ++i) {
    const auto prefix = "cameras[" + std::to_string(i) + "]";
    auto id = ReadU64(in, prefix + ".id");
    auto name = ReadString(in, prefix + ".name");
    if (!id.ok()) return id.status();
    if (!name.ok()) return name.status();
    for (const auto& field : {"fov", "near", "far"}) {
      auto value = ReadF32(in, prefix + "." + field);
      if (!value.ok()) return value.status();
    }
  }
  return Status::Ok();
}

Status InspectLights(InspectReader* in) {
  auto count = ReadU32(in, "lights.count", BinaryFieldKind::kCount);
  if (!count.ok()) return count.status();
  for (std::uint32_t i = 0; i < count.value(); ++i) {
    const auto prefix = "lights[" + std::to_string(i) + "]";
    auto id = ReadU64(in, prefix + ".id");
    auto name = ReadString(in, prefix + ".name");
    auto type = ReadU8(in, prefix + ".type", BinaryFieldKind::kEnum);
    if (!id.ok()) return id.status();
    if (!name.ok()) return name.status();
    if (!type.ok()) return type.status();
    for (const auto& field : {"color.r", "color.g", "color.b", "intensity"}) {
      auto value = ReadF32(in, prefix + "." + field);
      if (!value.ok()) return value.status();
    }
  }
  return Status::Ok();
}

Status InspectSkeletons(InspectReader* in) {
  auto count = ReadU32(in, "skeletons.count", BinaryFieldKind::kCount);
  if (!count.ok()) return count.status();
  for (std::uint32_t i = 0; i < count.value(); ++i) {
    const auto prefix = "skeletons[" + std::to_string(i) + "]";
    auto id = ReadU64(in, prefix + ".id");
    auto name = ReadString(in, prefix + ".name");
    auto joints = ReadU32(in, prefix + ".joints.count", BinaryFieldKind::kCount);
    if (!id.ok()) return id.status();
    if (!name.ok()) return name.status();
    if (!joints.ok()) return joints.status();
    for (std::uint32_t joint = 0; joint < joints.value(); ++joint) {
      const auto joint_prefix = prefix + ".joints[" + std::to_string(joint) + "]";
      auto joint_id = ReadU64(in, joint_prefix + ".id");
      auto joint_name = ReadString(in, joint_prefix + ".name");
      auto parent = ReadU64(in, joint_prefix + ".parent");
      auto transform = SkipTransform(in, joint_prefix + ".bind_pose");
      if (!joint_id.ok()) return joint_id.status();
      if (!joint_name.ok()) return joint_name.status();
      if (!parent.ok()) return parent.status();
      if (!transform.ok()) return transform;
    }
  }
  return Status::Ok();
}

Status InspectAnimations(InspectReader* in) {
  auto count = ReadU32(in, "animations.count", BinaryFieldKind::kCount);
  if (!count.ok()) return count.status();
  for (std::uint32_t i = 0; i < count.value(); ++i) {
    const auto prefix = "animations[" + std::to_string(i) + "]";
    auto id = ReadU64(in, prefix + ".id");
    auto name = ReadString(in, prefix + ".name");
    auto duration = ReadF32(in, prefix + ".duration");
    auto channels = ReadU32(in, prefix + ".channels.count", BinaryFieldKind::kCount);
    if (!id.ok()) return id.status();
    if (!name.ok()) return name.status();
    if (!duration.ok()) return duration.status();
    if (!channels.ok()) return channels.status();
    for (std::uint32_t channel = 0; channel < channels.value(); ++channel) {
      const auto channel_prefix = prefix + ".channels[" + std::to_string(channel) + "]";
      auto target = ReadU64(in, channel_prefix + ".target_node");
      auto keys = ReadU32(in, channel_prefix + ".keys.count", BinaryFieldKind::kCount);
      if (!target.ok()) return target.status();
      if (!keys.ok()) return keys.status();
      for (std::uint32_t key = 0; key < keys.value(); ++key) {
        const auto key_prefix = channel_prefix + ".keys[" + std::to_string(key) + "]";
        auto time = ReadF32(in, key_prefix + ".time");
        auto transform = SkipTransform(in, key_prefix + ".transform");
        if (!time.ok()) return time.status();
        if (!transform.ok()) return transform;
      }
    }
  }
  return Status::Ok();
}

}  // namespace

std::string BinaryFieldKindName(BinaryFieldKind kind) {
  switch (kind) {
    case BinaryFieldKind::kMagic:
      return "magic";
    case BinaryFieldKind::kVersion:
      return "version";
    case BinaryFieldKind::kFlags:
      return "flags";
    case BinaryFieldKind::kString:
      return "string";
    case BinaryFieldKind::kCount:
      return "count";
    case BinaryFieldKind::kObjectId:
      return "object_id";
    case BinaryFieldKind::kFloat:
      return "float";
    case BinaryFieldKind::kEnum:
      return "enum";
    case BinaryFieldKind::kPayload:
      return "payload";
  }
  return "unknown";
}

Result<BinaryInspection> InspectBinaryScene(const std::vector<std::uint8_t>& bytes) {
  InspectReader in(bytes);
  const auto magic_offset = in.reader.position();
  std::string magic;
  for (std::uint8_t expected : kSceneMagic) {
    auto byte = in.reader.ReadU8();
    if (!byte.ok()) return byte.status();
    magic.push_back(static_cast<char>(byte.value()));
    if (byte.value() != expected) {
      in.inspection.warnings.push_back("input does not start with BSEN magic");
    }
  }
  AddField(&in.inspection, BinaryFieldKind::kMagic, "magic", magic_offset, kSceneMagic.size(), magic);
  auto major = ReadU16(&in, "major", BinaryFieldKind::kVersion);
  auto minor = ReadU16(&in, "minor", BinaryFieldKind::kVersion);
  auto flags = ReadU32(&in, "flags", BinaryFieldKind::kFlags);
  if (!major.ok()) return major.status();
  if (!minor.ok()) return minor.status();
  if (!flags.ok()) return flags.status();
  in.inspection.recognized = major.value() == kFormatMajor;
  if (!in.inspection.recognized) {
    in.inspection.warnings.push_back("unsupported major version");
  }
  auto name = ReadString(&in, "scene.name");
  if (!name.ok()) return name.status();
  auto metadata_count = ReadU32(&in, "metadata.count", BinaryFieldKind::kCount);
  if (!metadata_count.ok()) return metadata_count.status();
  for (std::uint32_t i = 0; i < metadata_count.value(); ++i) {
    auto key = ReadString(&in, "metadata[" + std::to_string(i) + "].key");
    auto value = ReadString(&in, "metadata[" + std::to_string(i) + "].value");
    if (!key.ok()) return key.status();
    if (!value.ok()) return value.status();
  }
  for (Status status : {InspectNodes(&in), InspectTextures(&in), InspectMaterials(&in),
                        InspectMeshes(&in), InspectCameras(&in), InspectLights(&in),
                        InspectSkeletons(&in), InspectAnimations(&in)}) {
    if (!status.ok()) return status;
  }
  if (!in.reader.exhausted()) {
    in.inspection.warnings.push_back("trailing bytes remain after structured scene payload");
  }
  return in.inspection;
}

std::string FormatBinaryInspection(const BinaryInspection& inspection) {
  std::ostringstream out;
  out << "recognized=" << (inspection.recognized ? "true" : "false") << "\n";
  for (const auto& warning : inspection.warnings) {
    out << "warning: " << warning << "\n";
  }
  for (const auto& field : inspection.fields) {
    out << field.offset << "+" << field.size << " " << BinaryFieldKindName(field.kind) << " "
        << field.name << "=" << field.value << "\n";
  }
  return out.str();
}

}  // namespace bse
