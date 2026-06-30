#include "bse/compression.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

extern "C" int LLVMFuzzerTestOneInput(const std::uint8_t* data, std::size_t size) {
  const std::vector<std::uint8_t> input(data, data + size);
  auto compressed = bse::CompressRle(input);
  if (compressed.ok()) {
    auto decompressed = bse::DecompressRle(compressed.value());
    (void)decompressed;
  }
  auto arbitrary = bse::DecompressRle(input);
  (void)arbitrary;
  return 0;
}
