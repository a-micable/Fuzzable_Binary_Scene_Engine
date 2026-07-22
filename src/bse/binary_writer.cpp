#include "bse/binary_writer.hpp"

#include "bse/format.hpp"

#include <cstring>
#include <limits>

namespace bse {

void BinaryWriter::WriteU8(std::uint8_t value) { bytes_.push_back(value); }

void BinaryWriter::WriteU16(std::uint16_t value) { WriteUnsigned(value, 2); }

void BinaryWriter::WriteU32(std::uint32_t value) { WriteUnsigned(value, 4); }

void BinaryWriter::WriteU64(std::uint64_t value) { WriteUnsigned(value, 8); }

void BinaryWriter::WriteF32(float value) {
  std::uint32_t raw = 0;
  static_assert(sizeof(raw) == sizeof(value), "float must be 32 bits");
  std::memcpy(&raw, &value, sizeof(raw));
  WriteU32(raw);
}

Status BinaryWriter::WriteString(const std::string& value) {
  if (value.size() > kMaxStringBytes) {
    return Status::Error(ErrorCode::kLimitExceeded, "string is too large to serialize");
  }
  if (value.size() > std::numeric_limits<std::uint32_t>::max()) {
    return Status::Error(ErrorCode::kLimitExceeded, "string length does not fit in u32");
  }
  WriteU32(static_cast<std::uint32_t>(value.size()));
  bytes_.insert(bytes_.end(), value.begin(), value.end());
  return Status::Ok();
}

void BinaryWriter::WriteBytes(const std::vector<std::uint8_t>& bytes) {
  bytes_.insert(bytes_.end(), bytes.begin(), bytes.end());
}

void BinaryWriter::WriteUnsigned(std::uint64_t value, std::size_t bytes) {
  for (std::size_t i = 0; i < bytes; ++i) {
    bytes_.push_back(static_cast<std::uint8_t>((value >> (i * 8U)) & 0xFFU));
  }
}

}  // namespace bse
