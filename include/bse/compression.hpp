#pragma once

#include "bse/status.hpp"

#include <cstdint>
#include <vector>

namespace bse {

Result<std::vector<std::uint8_t>> CompressRle(const std::vector<std::uint8_t>& input);
Result<std::vector<std::uint8_t>> DecompressRle(const std::vector<std::uint8_t>& input);

}  // namespace bse
