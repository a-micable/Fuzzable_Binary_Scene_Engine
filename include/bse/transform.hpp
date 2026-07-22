#pragma once

#include "bse/scene.hpp"
#include "bse/status.hpp"

#include <array>
#include <unordered_map>

namespace bse {

struct Mat4 {
  std::array<float, 16> m{};
};

Mat4 IdentityMat4();
Mat4 TranslationMat4(const Vec3& value);
Mat4 ScaleMat4(const Vec3& value);
Mat4 RotationMat4(const Quat& value);
Mat4 TransformMat4(const Transform& transform);
Mat4 Multiply(const Mat4& lhs, const Mat4& rhs);
Vec3 TransformPoint(const Mat4& matrix, const Vec3& point);
Vec3 TransformVector(const Mat4& matrix, const Vec3& vector);
Result<std::unordered_map<ObjectId, Mat4>> ComputeWorldTransforms(const Scene& scene);
Result<Mat4> ComputeNodeWorldTransform(const Scene& scene, ObjectId node);
Transform ComposeTransform(const Transform& parent, const Transform& child);
Transform InverseApproximate(const Transform& transform);
Quat NormalizeQuat(Quat value);
Quat MultiplyQuat(const Quat& lhs, const Quat& rhs);

}  // namespace bse
