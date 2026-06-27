#pragma once
#include "bse/binary_scene_engine.hpp"
namespace bse::animation {
struct TimelineRange { float start=0.0f; float end=0.0f; bool empty=true; };
TimelineRange rangeOf(const AnimationClip& clip);
float wrapTime(float time, float duration);
}
