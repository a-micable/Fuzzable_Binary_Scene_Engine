#pragma once

#include "bse/scene.hpp"
#include "bse/status.hpp"

#include <vector>

namespace bse {

struct SampledChannel {
  ObjectId target_node = kInvalidObjectId;
  Transform transform;
  bool found = false;
};

struct AnimationBakeOptions {
  float sample_rate_hz = 30.0F;
  bool include_end_time = true;
};

struct BakedAnimation {
  std::string name;
  float duration_seconds = 0.0F;
  std::vector<float> times;
  std::vector<std::vector<SampledChannel>> frames;
};

float ClampTime(float time, float duration);
Vec3 Lerp(Vec3 lhs, Vec3 rhs, float t);
Quat Slerp(Quat lhs, Quat rhs, float t);
Transform InterpolateTransform(const Transform& lhs, const Transform& rhs, float t);
Result<SampledChannel> SampleChannel(const AnimationChannel& channel, float time);
Result<std::vector<SampledChannel>> SampleAnimation(const Animation& animation, float time);
Result<BakedAnimation> BakeAnimation(const Animation& animation,
                                     const AnimationBakeOptions& options = AnimationBakeOptions{});
Result<Scene> ApplyAnimationPose(const Scene& scene, const Animation& animation, float time);
Result<Animation> RetargetAnimation(const Animation& animation, ObjectId old_target,
                                    ObjectId new_target);
Result<Animation> ReverseAnimation(const Animation& animation);

}  // namespace bse
