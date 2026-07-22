#pragma once

#include "bse/status.hpp"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace bse {

class BinaryWriter {
 public:
  void WriteU8(std::uint8_t value);
  void WriteU16(std::uint16_t value);
  void WriteU32(std::uint32_t value);
  void WriteU64(std::uint64_t value);
  void WriteF32(float value);
  Status WriteString(const std::string& value);
  void WriteBytes(const std::vector<std::uint8_t>& bytes);

  const std::vector<std::uint8_t>& bytes() const { return bytes_; }
  std::vector<std::uint8_t> TakeBytes() { return std::move(bytes_); }

 private:
  void WriteUnsigned(std::uint64_t value, std::size_t bytes);

  std::vector<std::uint8_t> bytes_;
};

}  // namespace bse
