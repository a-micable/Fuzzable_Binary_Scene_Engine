#pragma once

#include "bse/status.hpp"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace bse {

class BinaryReader {
 public:
  BinaryReader(const std::uint8_t* data, std::size_t size);
  explicit BinaryReader(const std::vector<std::uint8_t>& data);

  std::size_t position() const { return position_; }
  std::size_t remaining() const { return size_ - position_; }
  bool exhausted() const { return remaining() == 0; }

  Result<std::uint8_t> ReadU8();
  Result<std::uint16_t> ReadU16();
  Result<std::uint32_t> ReadU32();
  Result<std::uint64_t> ReadU64();
  Result<float> ReadF32();
  Result<std::string> ReadString(std::size_t max_bytes);
  Result<std::vector<std::uint8_t>> ReadBytes(std::size_t count);
  Status Skip(std::size_t count);

 private:
  Result<std::uint64_t> ReadUnsigned(std::size_t bytes);

  const std::uint8_t* data_ = nullptr;
  std::size_t size_ = 0;
  std::size_t position_ = 0;
};

}  // namespace bse
