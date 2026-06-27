#pragma once
#include "bse/binary_scene_engine.hpp"
#include <unordered_map>
namespace bse::animation {
std::unordered_map<ObjectId, Transform> sampleClip(const AnimationClip& clip, float time);
void applySampledTransforms(Scene& scene, const std::unordered_map<ObjectId, Transform>& transforms);
}
