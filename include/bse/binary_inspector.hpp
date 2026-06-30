#pragma once

#include "bse/status.hpp"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace bse {

enum class BinaryFieldKind {
  kMagic,
  kVersion,
  kFlags,
  kString,
  kCount,
  kObjectId,
  kFloat,
  kEnum,
  kPayload
};

struct BinaryField {
  BinaryFieldKind kind = BinaryFieldKind::kPayload;
  std::string name;
  std::size_t offset = 0;
  std::size_t size = 0;
  std::string value;
};

struct BinaryInspection {
  bool recognized = false;
  std::vector<BinaryField> fields;
  std::vector<std::string> warnings;
};

std::string BinaryFieldKindName(BinaryFieldKind kind);
Result<BinaryInspection> InspectBinaryScene(const std::vector<std::uint8_t>& bytes);
std::string FormatBinaryInspection(const BinaryInspection& inspection);

}  // namespace bse
