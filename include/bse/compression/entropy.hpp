#pragma once
#include <cstdint>
#include <vector>
namespace bse::compression_ext {
double byteEntropy(const std::vector<std::uint8_t>& input);
bool looksCompressible(const std::vector<std::uint8_t>& input);
}
