#pragma once
#include "bse/binary_scene_engine.hpp"
namespace bse::math {
float clamp01(float value);
float lerp(float a, float b, float t);
Vec3 lerp(const Vec3& a, const Vec3& b, float t);
Vec4 normalizeQuat(const Vec4& q);
Vec4 nlerpQuat(const Vec4& a, const Vec4& b, float t);
}
