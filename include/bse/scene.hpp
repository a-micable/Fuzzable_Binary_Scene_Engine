#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace bse {

using ObjectId = std::uint64_t;
constexpr ObjectId kInvalidObjectId = 0;

struct Vec2 {
  float x = 0.0F;
  float y = 0.0F;
};

struct Vec3 {
  float x = 0.0F;
  float y = 0.0F;
  float z = 0.0F;
};

struct Quat {
  float x = 0.0F;
  float y = 0.0F;
  float z = 0.0F;
  float w = 1.0F;
};

struct Transform {
  Vec3 translation;
  Quat rotation;
  Vec3 scale{1.0F, 1.0F, 1.0F};
};

struct Texture {
  ObjectId id = kInvalidObjectId;
  std::string name;
  std::string uri;
  std::uint32_t width = 0;
  std::uint32_t height = 0;
  std::string color_space = "linear";
};

struct Material {
  ObjectId id = kInvalidObjectId;
  std::string name;
  Vec3 base_color{1.0F, 1.0F, 1.0F};
  float roughness = 1.0F;
  float metallic = 0.0F;
  ObjectId base_color_texture = kInvalidObjectId;
};

struct Vertex {
  Vec3 position;
  Vec3 normal;
  Vec2 texcoord;
};

struct Mesh {
  ObjectId id = kInvalidObjectId;
  std::string name;
  ObjectId material = kInvalidObjectId;
  std::vector<Vertex> vertices;
  std::vector<std::uint32_t> indices;
};

struct Camera {
  ObjectId id = kInvalidObjectId;
  std::string name;
  float vertical_fov_degrees = 60.0F;
  float near_plane = 0.01F;
  float far_plane = 1000.0F;
};

enum class LightType : std::uint8_t {
  kDirectional = 1,
  kPoint = 2,
  kSpot = 3
};

struct Light {
  ObjectId id = kInvalidObjectId;
  std::string name;
  LightType type = LightType::kPoint;
  Vec3 color{1.0F, 1.0F, 1.0F};
  float intensity = 1.0F;
};

struct SkeletonJoint {
  ObjectId id = kInvalidObjectId;
  std::string name;
  ObjectId parent = kInvalidObjectId;
  Transform bind_pose;
};

struct Skeleton {
  ObjectId id = kInvalidObjectId;
  std::string name;
  std::vector<SkeletonJoint> joints;
};

struct AnimationKey {
  float time_seconds = 0.0F;
  Transform transform;
};

struct AnimationChannel {
  ObjectId target_node = kInvalidObjectId;
  std::vector<AnimationKey> keys;
};

struct Animation {
  ObjectId id = kInvalidObjectId;
  std::string name;
  float duration_seconds = 0.0F;
  std::vector<AnimationChannel> channels;
};

struct Node {
  ObjectId id = kInvalidObjectId;
  std::string name;
  ObjectId parent = kInvalidObjectId;
  Transform local;
  ObjectId mesh = kInvalidObjectId;
  ObjectId camera = kInvalidObjectId;
  ObjectId light = kInvalidObjectId;
  std::vector<ObjectId> children;
};

struct Scene {
  std::string name = "Untitled";
  std::unordered_map<std::string, std::string> metadata;
  std::vector<Node> nodes;
  std::vector<Mesh> meshes;
  std::vector<Material> materials;
  std::vector<Texture> textures;
  std::vector<Animation> animations;
  std::vector<Skeleton> skeletons;
  std::vector<Camera> cameras;
  std::vector<Light> lights;
};

Scene MakeMinimalScene(std::string name);

}  // namespace bse
