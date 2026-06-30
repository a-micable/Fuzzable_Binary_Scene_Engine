#pragma once

#include "bse/mesh_tools.hpp"
#include "bse/scene.hpp"
#include "bse/status.hpp"

#include <string>
#include <vector>

namespace bse {

struct SceneBuilderOptions {
  std::string name = "generated";
  bool add_default_camera = true;
  bool add_default_light = true;
  bool add_metadata = true;
};

struct MaterialDescriptor {
  std::string name;
  Vec3 base_color{1.0F, 1.0F, 1.0F};
  float roughness = 1.0F;
  float metallic = 0.0F;
};

class SceneBuilder {
 public:
  explicit SceneBuilder(SceneBuilderOptions options = SceneBuilderOptions{});

  ObjectId AddNode(std::string name, ObjectId parent = kInvalidObjectId);
  ObjectId AddMeshNode(std::string name, Mesh mesh, ObjectId parent = kInvalidObjectId);
  ObjectId AddMaterial(MaterialDescriptor descriptor);
  ObjectId AddTexture(std::string name, std::string uri, std::uint32_t width, std::uint32_t height,
                      std::string color_space = "srgb");
  ObjectId AddCamera(std::string name, float fov, float near_plane, float far_plane,
                     ObjectId parent = kInvalidObjectId);
  ObjectId AddLight(std::string name, LightType type, Vec3 color, float intensity,
                    ObjectId parent = kInvalidObjectId);
  ObjectId AddLiftAnimation(std::string name, ObjectId target_node, float height,
                            float duration_seconds);
  void SetNodeTransform(ObjectId node, const Transform& transform);
  void AddMetadata(std::string key, std::string value);
  Result<Scene> Build();

 private:
  ObjectId Allocate();
  Node* FindNode(ObjectId id);

  Scene scene_;
  ObjectId next_id_ = 1;
};

Scene MakeGridScene(std::uint32_t columns, std::uint32_t rows);
Scene MakePrimitiveShowcaseScene();
Scene MakeMaterialPaletteScene(const std::vector<MaterialDescriptor>& materials);
Result<Scene> MakeSceneFromMeshes(std::string name, const std::vector<Mesh>& meshes);

}  // namespace bse
