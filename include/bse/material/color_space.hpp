#pragma once
#include "bse/binary_scene_engine.hpp"
namespace bse::material {
float srgbToLinear(float value);
float linearToSrgb(float value);
Vec4 srgbToLinear(Vec4 color);
Vec4 linearToSrgb(Vec4 color);
}
