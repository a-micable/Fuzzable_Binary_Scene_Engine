#pragma once

#include "bse/scene.hpp"

#include <cstddef>
#include <cstdint>

namespace bse_fuzz {

inline bse::Scene SceneFromBytes(const std::uint8_t* data, std::size_t size) {
  bse::Scene scene = bse::MakeMinimalScene("fuzz");
  if (size >= 3) {
    scene.nodes.front().local.translation.x = static_cast<float>(data[0] % 17U);
    scene.nodes.front().local.translation.y = static_cast<float>(data[1] % 17U);
    scene.nodes.front().local.translation.z = static_cast<float>(data[2] % 17U);
  }
  if (size >= 12) {
    bse::Texture texture;
    texture.id = 10;
    texture.name = "fuzz_texture";
    texture.uri = "memory://texture";
    texture.width = data[3] + 1U;
    texture.height = data[4] + 1U;
    scene.textures.push_back(texture);

    bse::Material material;
    material.id = 20;
    material.name = "fuzz_material";
    material.base_color = {data[5] / 255.0F, data[6] / 255.0F, data[7] / 255.0F};
    material.roughness = data[8] / 255.0F;
    material.base_color_texture = 10;
    scene.materials.push_back(material);

    bse::Mesh mesh;
    mesh.id = 30;
    mesh.name = "fuzz_mesh";
    mesh.material = 20;
    mesh.vertices.push_back({{0.0F, 0.0F, 0.0F}, {0.0F, 1.0F, 0.0F}, {0.0F, 0.0F}});
    mesh.vertices.push_back({{1.0F, 0.0F, 0.0F}, {0.0F, 1.0F, 0.0F}, {1.0F, 0.0F}});
    mesh.vertices.push_back({{0.0F, 1.0F, 0.0F}, {0.0F, 1.0F, 0.0F}, {0.0F, 1.0F}});
    mesh.indices = {0, 1, 2};
    scene.meshes.push_back(mesh);
    scene.nodes.front().mesh = 30;
  }
  return scene;
}

}  // namespace bse_fuzz
