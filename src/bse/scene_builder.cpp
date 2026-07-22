#include "bse/scene_builder.hpp"

#include "bse/scene_ops.hpp"
#include "bse/validator.hpp"

#include <algorithm>
#include <utility>

namespace bse {

SceneBuilder::SceneBuilder(SceneBuilderOptions options) {
  scene_.name = std::move(options.name);
  if (options.add_metadata) {
    scene_.metadata["generator"] = "Binary Scene Engine";
    scene_.metadata["profile"] = "procedural";
  }
  Node root;
  root.id = Allocate();
  root.name = "root";
  scene_.nodes.push_back(root);
  if (options.add_default_camera) {
    AddCamera("main_camera", 60.0F, 0.05F, 500.0F, root.id);
  }
  if (options.add_default_light) {
    AddLight("key_light", LightType::kDirectional, {1.0F, 0.96F, 0.9F}, 2.5F, root.id);
  }
}

ObjectId SceneBuilder::Allocate() { return next_id_++; }

Node* SceneBuilder::FindNode(ObjectId id) {
  for (auto& node : scene_.nodes) {
    if (node.id == id) {
      return &node;
    }
  }
  return nullptr;
}

ObjectId SceneBuilder::AddNode(std::string name, ObjectId parent) {
  Node node;
  node.id = Allocate();
  node.name = std::move(name);
  node.parent = parent;
  scene_.nodes.push_back(node);
  if (auto* parent_node = FindNode(parent)) {
    parent_node->children.push_back(node.id);
  }
  return node.id;
}

ObjectId SceneBuilder::AddMeshNode(std::string name, Mesh mesh, ObjectId parent) {
  if (parent == kInvalidObjectId && !scene_.nodes.empty()) {
    parent = scene_.nodes.front().id;
  }
  mesh.id = Allocate();
  const ObjectId mesh_id = mesh.id;
  scene_.meshes.push_back(std::move(mesh));
  const ObjectId node_id = AddNode(std::move(name), parent);
  if (auto* node = FindNode(node_id)) {
    node->mesh = mesh_id;
  }
  return node_id;
}

ObjectId SceneBuilder::AddMaterial(MaterialDescriptor descriptor) {
  Material material;
  material.id = Allocate();
  material.name = std::move(descriptor.name);
  material.base_color = descriptor.base_color;
  material.roughness = descriptor.roughness;
  material.metallic = descriptor.metallic;
  scene_.materials.push_back(material);
  return material.id;
}

ObjectId SceneBuilder::AddTexture(std::string name, std::string uri, std::uint32_t width,
                                  std::uint32_t height, std::string color_space) {
  Texture texture;
  texture.id = Allocate();
  texture.name = std::move(name);
  texture.uri = std::move(uri);
  texture.width = width;
  texture.height = height;
  texture.color_space = std::move(color_space);
  scene_.textures.push_back(texture);
  return texture.id;
}

ObjectId SceneBuilder::AddCamera(std::string name, float fov, float near_plane, float far_plane,
                                 ObjectId parent) {
  Camera camera;
  camera.id = Allocate();
  camera.name = name;
  camera.vertical_fov_degrees = fov;
  camera.near_plane = near_plane;
  camera.far_plane = far_plane;
  scene_.cameras.push_back(camera);
  const ObjectId node_id = AddNode(std::move(name), parent);
  if (auto* node = FindNode(node_id)) {
    node->camera = camera.id;
    node->local.translation = {0.0F, -4.0F, 2.0F};
  }
  return camera.id;
}

ObjectId SceneBuilder::AddLight(std::string name, LightType type, Vec3 color, float intensity,
                                ObjectId parent) {
  Light light;
  light.id = Allocate();
  light.name = name;
  light.type = type;
  light.color = color;
  light.intensity = intensity;
  scene_.lights.push_back(light);
  const ObjectId node_id = AddNode(std::move(name), parent);
  if (auto* node = FindNode(node_id)) {
    node->light = light.id;
    node->local.translation = {0.0F, -1.0F, 3.0F};
  }
  return light.id;
}

ObjectId SceneBuilder::AddLiftAnimation(std::string name, ObjectId target_node, float height,
                                        float duration_seconds) {
  Animation animation;
  animation.id = Allocate();
  animation.name = std::move(name);
  animation.duration_seconds = duration_seconds;
  AnimationChannel channel;
  channel.target_node = target_node;
  channel.keys.push_back({0.0F, {}});
  Transform lifted;
  lifted.translation = {0.0F, height, 0.0F};
  channel.keys.push_back({duration_seconds, lifted});
  animation.channels.push_back(channel);
  scene_.animations.push_back(animation);
  return animation.id;
}

void SceneBuilder::SetNodeTransform(ObjectId node_id, const Transform& transform) {
  if (auto* node = FindNode(node_id)) {
    node->local = transform;
  }
}

void SceneBuilder::AddMetadata(std::string key, std::string value) {
  scene_.metadata[std::move(key)] = std::move(value);
}

Result<Scene> SceneBuilder::Build() {
  NormalizeScene(&scene_);
  auto status = ValidateScene(scene_);
  if (!status.ok()) {
    return status;
  }
  return scene_;
}

Scene MakeGridScene(std::uint32_t columns, std::uint32_t rows) {
  SceneBuilder builder({ "grid", true, true, true });
  const auto material = builder.AddMaterial({"grid_matte", {0.55F, 0.62F, 0.7F}, 0.75F, 0.0F});
  MeshBuildOptions options;
  options.name = "grid";
  options.material = material;
  auto mesh = MakePlaneMesh(static_cast<float>(columns), static_cast<float>(rows), columns, rows, options);
  builder.AddMeshNode("grid", std::move(mesh));
  auto scene = builder.Build();
  return scene.ok() ? scene.value() : Scene{};
}

Scene MakePrimitiveShowcaseScene() {
  SceneBuilder builder({ "primitive-showcase", true, true, true });
  const auto red = builder.AddMaterial({"red_matte", {0.85F, 0.15F, 0.1F}, 0.65F, 0.0F});
  const auto green = builder.AddMaterial({"green_matte", {0.15F, 0.75F, 0.25F}, 0.55F, 0.0F});
  const auto blue = builder.AddMaterial({"blue_matte", {0.2F, 0.3F, 0.9F}, 0.45F, 0.0F});

  MeshBuildOptions box_options;
  box_options.name = "box";
  box_options.material = red;
  auto box_node = builder.AddMeshNode("box", MakeBoxMesh(1.0F, 1.0F, 1.0F, box_options));
  Transform box_transform;
  box_transform.translation = {-1.5F, 0.5F, 0.0F};
  builder.SetNodeTransform(box_node, box_transform);

  MeshBuildOptions sphere_options;
  sphere_options.name = "sphere";
  sphere_options.material = green;
  auto sphere_node =
      builder.AddMeshNode("sphere", MakeUvSphereMesh(0.65F, 24, 12, sphere_options));
  Transform sphere_transform;
  sphere_transform.translation = {0.0F, 0.65F, 0.0F};
  builder.SetNodeTransform(sphere_node, sphere_transform);

  MeshBuildOptions cylinder_options;
  cylinder_options.name = "cylinder";
  cylinder_options.material = blue;
  auto cylinder_node =
      builder.AddMeshNode("cylinder", MakeCylinderMesh(0.45F, 1.4F, 24, cylinder_options));
  Transform cylinder_transform;
  cylinder_transform.translation = {1.5F, 0.7F, 0.0F};
  builder.SetNodeTransform(cylinder_node, cylinder_transform);
  builder.AddLiftAnimation("sphere_lift", sphere_node, 0.5F, 1.0F);
  auto scene = builder.Build();
  return scene.ok() ? scene.value() : Scene{};
}

Scene MakeMaterialPaletteScene(const std::vector<MaterialDescriptor>& materials) {
  SceneBuilder builder({ "material-palette", true, true, true });
  float x = 0.0F;
  for (const auto& descriptor : materials) {
    const auto material = builder.AddMaterial(descriptor);
    MeshBuildOptions options;
    options.name = descriptor.name + "_swatch";
    options.material = material;
    auto node = builder.AddMeshNode(options.name, MakeBoxMesh(0.8F, 0.8F, 0.8F, options));
    Transform transform;
    transform.translation = {x, 0.4F, 0.0F};
    builder.SetNodeTransform(node, transform);
    x += 1.1F;
  }
  auto scene = builder.Build();
  return scene.ok() ? scene.value() : Scene{};
}

Result<Scene> MakeSceneFromMeshes(std::string name, const std::vector<Mesh>& meshes) {
  SceneBuilder builder({std::move(name), true, true, true});
  for (auto mesh : meshes) {
    builder.AddMeshNode(mesh.name, std::move(mesh));
  }
  return builder.Build();
}

}  // namespace bse
