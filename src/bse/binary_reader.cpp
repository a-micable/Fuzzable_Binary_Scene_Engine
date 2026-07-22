#include "bse/binary_reader.hpp"

#include <cstring>
#include <limits>

namespace bse {

BinaryReader::BinaryReader(const std::uint8_t* data, std::size_t size)
    : data_(data), size_(size) {}

BinaryReader::BinaryReader(const std::vector<std::uint8_t>& data)
    : BinaryReader(data.data(), data.size()) {}

Result<std::uint8_t> BinaryReader::ReadU8() {
  auto value = ReadUnsigned(1);
  if (!value.ok()) {
    return value.status();
  }
  return static_cast<std::uint8_t>(value.value());
}

Result<std::uint16_t> BinaryReader::ReadU16() {
  auto value = ReadUnsigned(2);
  if (!value.ok()) {
    return value.status();
  }
  return static_cast<std::uint16_t>(value.value());
}

Result<std::uint32_t> BinaryReader::ReadU32() {
  auto value = ReadUnsigned(4);
  if (!value.ok()) {
    return value.status();
  }
  return static_cast<std::uint32_t>(value.value());
}

Result<std::uint64_t> BinaryReader::ReadU64() { return ReadUnsigned(8); }

Result<float> BinaryReader::ReadF32() {
  auto bits = ReadU32();
  if (!bits.ok()) {
    return bits.status();
  }
  float value = 0.0F;
  static_assert(sizeof(value) == sizeof(bits.value()), "float must be 32 bits");
  std::uint32_t raw = bits.value();
  std::memcpy(&value, &raw, sizeof(value));
  return value;
}

Result<std::string> BinaryReader::ReadString(std::size_t max_bytes) {
  auto length = ReadU32();
  if (!length.ok()) {
    return length.status();
  }
  if (length.value() > max_bytes) {
    return Status::Error(ErrorCode::kLimitExceeded, "string length exceeds configured maximum");
  }
  if (remaining() < length.value()) {
    return Status::Error(ErrorCode::kUnexpectedEof, "string payload extends past end of buffer");
  }
  std::string value(reinterpret_cast<const char*>(data_ + position_), length.value());
  position_ += length.value();
  return value;
}

Result<std::vector<std::uint8_t>> BinaryReader::ReadBytes(std::size_t count) {
  if (remaining() < count) {
    return Status::Error(ErrorCode::kUnexpectedEof, "byte range extends past end of buffer");
  }
  std::vector<std::uint8_t> out(data_ + position_, data_ + position_ + count);
  position_ += count;
  return out;
}

Status BinaryReader::Skip(std::size_t count) {
  if (remaining() < count) {
    return Status::Error(ErrorCode::kUnexpectedEof, "skip extends past end of buffer");
  }
  position_ += count;
  return Status::Ok();
}

Result<std::uint64_t> BinaryReader::ReadUnsigned(std::size_t bytes) {
  if (bytes == 0 || bytes > sizeof(std::uint64_t)) {
    return Status::Error(ErrorCode::kMalformedData, "invalid integer width");
  }
  if (remaining() < bytes) {
    return Status::Error(ErrorCode::kUnexpectedEof, "integer extends past end of buffer");
  }
  std::uint64_t value = 0;
  for (std::size_t i = 0; i < bytes; ++i) {
    value |= static_cast<std::uint64_t>(data_[position_ + i]) << (i * 8U);
  }
  position_ += bytes;
  return value;
}

}  // namespace bse
