#include "bse/transform.hpp"

#include "bse/validator.hpp"

#include <cmath>
#include <unordered_set>

namespace bse {

namespace {

constexpr float kEpsilon = 0.000001F;

float Dot(const Quat& q) {
  return q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w;
}

Vec3 Add(const Vec3& lhs, const Vec3& rhs) {
  return {lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z};
}

Vec3 Mul(const Vec3& lhs, const Vec3& rhs) {
  return {lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z};
}

Vec3 SafeReciprocal(const Vec3& value) {
  return {std::fabs(value.x) < kEpsilon ? 0.0F : 1.0F / value.x,
          std::fabs(value.y) < kEpsilon ? 0.0F : 1.0F / value.y,
          std::fabs(value.z) < kEpsilon ? 0.0F : 1.0F / value.z};
}

const Node* FindNode(const Scene& scene, ObjectId id) {
  for (const auto& node : scene.nodes) {
    if (node.id == id) {
      return &node;
    }
  }
  return nullptr;
}

Result<Mat4> ComputeRecursive(const Scene& scene, ObjectId id, std::unordered_set<ObjectId>* visiting,
                              std::unordered_map<ObjectId, Mat4>* cache) {
  auto cached = cache->find(id);
  if (cached != cache->end()) {
    return cached->second;
  }
  const Node* node = FindNode(scene, id);
  if (node == nullptr) {
    return Status::Error(ErrorCode::kMissingReference, "node not found while computing transform");
  }
  if (!visiting->insert(id).second) {
    return Status::Error(ErrorCode::kMalformedData, "cycle while computing node transform");
  }
  Mat4 local = TransformMat4(node->local);
  Mat4 world = local;
  if (node->parent != kInvalidObjectId) {
    auto parent = ComputeRecursive(scene, node->parent, visiting, cache);
    if (!parent.ok()) {
      visiting->erase(id);
      return parent.status();
    }
    world = Multiply(parent.value(), local);
  }
  visiting->erase(id);
  (*cache)[id] = world;
  return world;
}

}  // namespace

Mat4 IdentityMat4() {
  Mat4 out;
  out.m[0] = 1.0F;
  out.m[5] = 1.0F;
  out.m[10] = 1.0F;
  out.m[15] = 1.0F;
  return out;
}

Mat4 TranslationMat4(const Vec3& value) {
  Mat4 out = IdentityMat4();
  out.m[12] = value.x;
  out.m[13] = value.y;
  out.m[14] = value.z;
  return out;
}

Mat4 ScaleMat4(const Vec3& value) {
  Mat4 out = IdentityMat4();
  out.m[0] = value.x;
  out.m[5] = value.y;
  out.m[10] = value.z;
  return out;
}

Quat NormalizeQuat(Quat value) {
  const float length_squared = Dot(value);
  if (length_squared < kEpsilon) {
    return {};
  }
  const float inv_length = 1.0F / std::sqrt(length_squared);
  value.x *= inv_length;
  value.y *= inv_length;
  value.z *= inv_length;
  value.w *= inv_length;
  return value;
}

Quat MultiplyQuat(const Quat& lhs, const Quat& rhs) {
  return {lhs.w * rhs.x + lhs.x * rhs.w + lhs.y * rhs.z - lhs.z * rhs.y,
          lhs.w * rhs.y - lhs.x * rhs.z + lhs.y * rhs.w + lhs.z * rhs.x,
          lhs.w * rhs.z + lhs.x * rhs.y - lhs.y * rhs.x + lhs.z * rhs.w,
          lhs.w * rhs.w - lhs.x * rhs.x - lhs.y * rhs.y - lhs.z * rhs.z};
}

Mat4 RotationMat4(const Quat& value) {
  const Quat q = NormalizeQuat(value);
  const float xx = q.x * q.x;
  const float yy = q.y * q.y;
  const float zz = q.z * q.z;
  const float xy = q.x * q.y;
  const float xz = q.x * q.z;
  const float yz = q.y * q.z;
  const float wx = q.w * q.x;
  const float wy = q.w * q.y;
  const float wz = q.w * q.z;
  Mat4 out = IdentityMat4();
  out.m[0] = 1.0F - 2.0F * (yy + zz);
  out.m[1] = 2.0F * (xy + wz);
  out.m[2] = 2.0F * (xz - wy);
  out.m[4] = 2.0F * (xy - wz);
  out.m[5] = 1.0F - 2.0F * (xx + zz);
  out.m[6] = 2.0F * (yz + wx);
  out.m[8] = 2.0F * (xz + wy);
  out.m[9] = 2.0F * (yz - wx);
  out.m[10] = 1.0F - 2.0F * (xx + yy);
  return out;
}

Mat4 TransformMat4(const Transform& transform) {
  return Multiply(Multiply(TranslationMat4(transform.translation), RotationMat4(transform.rotation)),
                  ScaleMat4(transform.scale));
}

Mat4 Multiply(const Mat4& lhs, const Mat4& rhs) {
  Mat4 out;
  for (int column = 0; column < 4; ++column) {
    for (int row = 0; row < 4; ++row) {
      float sum = 0.0F;
      for (int k = 0; k < 4; ++k) {
        sum += lhs.m[static_cast<std::size_t>(k * 4 + row)] *
               rhs.m[static_cast<std::size_t>(column * 4 + k)];
      }
      out.m[static_cast<std::size_t>(column * 4 + row)] = sum;
    }
  }
  return out;
}

Vec3 TransformPoint(const Mat4& matrix, const Vec3& point) {
  return {matrix.m[0] * point.x + matrix.m[4] * point.y + matrix.m[8] * point.z + matrix.m[12],
          matrix.m[1] * point.x + matrix.m[5] * point.y + matrix.m[9] * point.z + matrix.m[13],
          matrix.m[2] * point.x + matrix.m[6] * point.y + matrix.m[10] * point.z + matrix.m[14]};
}

Vec3 TransformVector(const Mat4& matrix, const Vec3& vector) {
  return {matrix.m[0] * vector.x + matrix.m[4] * vector.y + matrix.m[8] * vector.z,
          matrix.m[1] * vector.x + matrix.m[5] * vector.y + matrix.m[9] * vector.z,
          matrix.m[2] * vector.x + matrix.m[6] * vector.y + matrix.m[10] * vector.z};
}

Result<std::unordered_map<ObjectId, Mat4>> ComputeWorldTransforms(const Scene& scene) {
  auto status = ValidateScene(scene);
  if (!status.ok()) {
    return status;
  }
  std::unordered_map<ObjectId, Mat4> cache;
  std::unordered_set<ObjectId> visiting;
  for (const auto& node : scene.nodes) {
    auto world = ComputeRecursive(scene, node.id, &visiting, &cache);
    if (!world.ok()) {
      return world.status();
    }
  }
  return cache;
}

Result<Mat4> ComputeNodeWorldTransform(const Scene& scene, ObjectId node) {
  auto transforms = ComputeWorldTransforms(scene);
  if (!transforms.ok()) {
    return transforms.status();
  }
  auto it = transforms.value().find(node);
  if (it == transforms.value().end()) {
    return Status::Error(ErrorCode::kMissingReference, "node transform not found");
  }
  return it->second;
}

Transform ComposeTransform(const Transform& parent, const Transform& child) {
  Transform out;
  out.translation = Add(parent.translation, Mul(parent.scale, child.translation));
  out.rotation = NormalizeQuat(MultiplyQuat(parent.rotation, child.rotation));
  out.scale = Mul(parent.scale, child.scale);
  return out;
}

Transform InverseApproximate(const Transform& transform) {
  Transform out;
  out.scale = SafeReciprocal(transform.scale);
  out.rotation = {-transform.rotation.x, -transform.rotation.y, -transform.rotation.z,
                  transform.rotation.w};
  out.rotation = NormalizeQuat(out.rotation);
  out.translation = {-transform.translation.x * out.scale.x, -transform.translation.y * out.scale.y,
                     -transform.translation.z * out.scale.z};
  return out;
}

}  // namespace bse
