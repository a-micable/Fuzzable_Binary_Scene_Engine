#include "bse/animation_tools.hpp"

#include "bse/transform.hpp"
#include "bse/validator.hpp"

#include <algorithm>
#include <cmath>

namespace bse {

namespace {

constexpr float kEpsilon = 0.000001F;

float Clamp01(float value) {
  if (value < 0.0F) return 0.0F;
  if (value > 1.0F) return 1.0F;
  return value;
}

float DotQuat(const Quat& lhs, const Quat& rhs) {
  return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z + lhs.w * rhs.w;
}

Quat Negate(Quat value) {
  value.x = -value.x;
  value.y = -value.y;
  value.z = -value.z;
  value.w = -value.w;
  return value;
}

Quat LerpQuat(Quat lhs, Quat rhs, float t) {
  return NormalizeQuat({lhs.x + (rhs.x - lhs.x) * t, lhs.y + (rhs.y - lhs.y) * t,
                        lhs.z + (rhs.z - lhs.z) * t, lhs.w + (rhs.w - lhs.w) * t});
}

Node* FindNode(Scene* scene, ObjectId id) {
  for (auto& node : scene->nodes) {
    if (node.id == id) {
      return &node;
    }
  }
  return nullptr;
}

}  // namespace

float ClampTime(float time, float duration) {
  if (time < 0.0F) {
    return 0.0F;
  }
  if (time > duration) {
    return duration;
  }
  return time;
}

Vec3 Lerp(Vec3 lhs, Vec3 rhs, float t) {
  t = Clamp01(t);
  return {lhs.x + (rhs.x - lhs.x) * t, lhs.y + (rhs.y - lhs.y) * t,
          lhs.z + (rhs.z - lhs.z) * t};
}

Quat Slerp(Quat lhs, Quat rhs, float t) {
  t = Clamp01(t);
  lhs = NormalizeQuat(lhs);
  rhs = NormalizeQuat(rhs);
  float dot = DotQuat(lhs, rhs);
  if (dot < 0.0F) {
    rhs = Negate(rhs);
    dot = -dot;
  }
  if (dot > 0.9995F) {
    return LerpQuat(lhs, rhs, t);
  }
  const float theta_0 = std::acos(dot);
  const float theta = theta_0 * t;
  const float sin_theta = std::sin(theta);
  const float sin_theta_0 = std::sin(theta_0);
  if (std::fabs(sin_theta_0) < kEpsilon) {
    return lhs;
  }
  const float s0 = std::cos(theta) - dot * sin_theta / sin_theta_0;
  const float s1 = sin_theta / sin_theta_0;
  return NormalizeQuat({lhs.x * s0 + rhs.x * s1, lhs.y * s0 + rhs.y * s1,
                        lhs.z * s0 + rhs.z * s1, lhs.w * s0 + rhs.w * s1});
}

Transform InterpolateTransform(const Transform& lhs, const Transform& rhs, float t) {
  Transform out;
  out.translation = Lerp(lhs.translation, rhs.translation, t);
  out.rotation = Slerp(lhs.rotation, rhs.rotation, t);
  out.scale = Lerp(lhs.scale, rhs.scale, t);
  return out;
}

Result<SampledChannel> SampleChannel(const AnimationChannel& channel, float time) {
  SampledChannel sampled;
  sampled.target_node = channel.target_node;
  if (channel.keys.empty()) {
    return sampled;
  }
  if (time <= channel.keys.front().time_seconds) {
    sampled.transform = channel.keys.front().transform;
    sampled.found = true;
    return sampled;
  }
  if (time >= channel.keys.back().time_seconds) {
    sampled.transform = channel.keys.back().transform;
    sampled.found = true;
    return sampled;
  }
  for (std::size_t i = 0; i + 1U < channel.keys.size(); ++i) {
    const auto& a = channel.keys[i];
    const auto& b = channel.keys[i + 1U];
    if (time >= a.time_seconds && time <= b.time_seconds) {
      const float span = b.time_seconds - a.time_seconds;
      const float t = span <= kEpsilon ? 0.0F : (time - a.time_seconds) / span;
      sampled.transform = InterpolateTransform(a.transform, b.transform, t);
      sampled.found = true;
      return sampled;
    }
  }
  return sampled;
}

Result<std::vector<SampledChannel>> SampleAnimation(const Animation& animation, float time) {
  time = ClampTime(time, animation.duration_seconds);
  std::vector<SampledChannel> samples;
  samples.reserve(animation.channels.size());
  for (const auto& channel : animation.channels) {
    auto sample = SampleChannel(channel, time);
    if (!sample.ok()) {
      return sample.status();
    }
    samples.push_back(sample.value());
  }
  return samples;
}

Result<BakedAnimation> BakeAnimation(const Animation& animation, const AnimationBakeOptions& options) {
  if (options.sample_rate_hz <= 0.0F) {
    return Status::Error(ErrorCode::kValidationFailed, "sample rate must be positive");
  }
  BakedAnimation baked;
  baked.name = animation.name;
  baked.duration_seconds = animation.duration_seconds;
  const float step = 1.0F / options.sample_rate_hz;
  for (float time = 0.0F; time < animation.duration_seconds; time += step) {
    auto samples = SampleAnimation(animation, time);
    if (!samples.ok()) {
      return samples.status();
    }
    baked.times.push_back(time);
    baked.frames.push_back(samples.value());
  }
  if (options.include_end_time || baked.times.empty()) {
    auto samples = SampleAnimation(animation, animation.duration_seconds);
    if (!samples.ok()) {
      return samples.status();
    }
    baked.times.push_back(animation.duration_seconds);
    baked.frames.push_back(samples.value());
  }
  return baked;
}

Result<Scene> ApplyAnimationPose(const Scene& scene, const Animation& animation, float time) {
  Scene posed = scene;
  auto samples = SampleAnimation(animation, time);
  if (!samples.ok()) {
    return samples.status();
  }
  for (const auto& sample : samples.value()) {
    if (!sample.found) {
      continue;
    }
    auto* node = FindNode(&posed, sample.target_node);
    if (node == nullptr) {
      return Status::Error(ErrorCode::kMissingReference, "animation target node not found");
    }
    node->local = sample.transform;
  }
  auto status = ValidateScene(posed);
  if (!status.ok()) {
    return status;
  }
  return posed;
}

Result<Animation> RetargetAnimation(const Animation& animation, ObjectId old_target,
                                    ObjectId new_target) {
  Animation retargeted = animation;
  bool changed = false;
  for (auto& channel : retargeted.channels) {
    if (channel.target_node == old_target) {
      channel.target_node = new_target;
      changed = true;
    }
  }
  if (!changed) {
    return Status::Error(ErrorCode::kMissingReference, "animation target was not found");
  }
  return retargeted;
}

Result<Animation> ReverseAnimation(const Animation& animation) {
  Animation reversed = animation;
  for (auto& channel : reversed.channels) {
    for (auto& key : channel.keys) {
      key.time_seconds = animation.duration_seconds - key.time_seconds;
    }
    std::reverse(channel.keys.begin(), channel.keys.end());
  }
  return reversed;
}

}  // namespace bse
