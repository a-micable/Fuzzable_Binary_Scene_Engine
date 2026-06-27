#pragma once
#include "bse/binary_scene_engine.hpp"
namespace bse::material {
struct ParameterBlock { Vec4 baseColor; float metallic; float roughness; ObjectId baseColorTexture; };
ParameterBlock parametersFor(const Material& material);
bool isOpaque(const ParameterBlock& block);
}
