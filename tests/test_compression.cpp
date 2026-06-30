#include "bse/compression.hpp"

#include "test_support.hpp"

namespace {

void RoundTripsRunsAndSingletons() {
  const std::vector<std::uint8_t> input{7, 7, 7, 1, 2, 2, 3, 3, 3, 3};
  auto compressed = bse::CompressRle(input);
  BSE_EXPECT_TRUE(compressed.ok());

  auto decompressed = bse::DecompressRle(compressed.value());
  BSE_EXPECT_TRUE(decompressed.ok());
  BSE_EXPECT_EQ(decompressed.value(), input);
}

void RejectsOddSizedStreams() {
  const std::vector<std::uint8_t> malformed{3, 9, 1};
  auto decompressed = bse::DecompressRle(malformed);
  BSE_EXPECT_TRUE(!decompressed.ok());
  BSE_EXPECT_EQ(decompressed.status().code(), bse::ErrorCode::kMalformedData);
}

}  // namespace

int main() {
  RoundTripsRunsAndSingletons();
  RejectsOddSizedStreams();
  return 0;
}
