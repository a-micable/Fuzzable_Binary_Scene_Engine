#include "bse/binary_reader.hpp"
#include "bse/binary_writer.hpp"

#include "test_support.hpp"

namespace {

void RoundTripsLittleEndianNumbers() {
  bse::BinaryWriter writer;
  writer.WriteU8(7);
  writer.WriteU16(0x1234);
  writer.WriteU32(0x12345678);
  writer.WriteU64(0x1122334455667788ULL);
  writer.WriteF32(3.5F);

  bse::BinaryReader reader(writer.bytes());
  BSE_EXPECT_EQ(reader.ReadU8().value(), 7);
  BSE_EXPECT_EQ(reader.ReadU16().value(), 0x1234);
  BSE_EXPECT_EQ(reader.ReadU32().value(), 0x12345678U);
  BSE_EXPECT_EQ(reader.ReadU64().value(), 0x1122334455667788ULL);
  BSE_EXPECT_EQ(reader.ReadF32().value(), 3.5F);
  BSE_EXPECT_TRUE(reader.exhausted());
}

void RejectsTruncatedReads() {
  const std::vector<std::uint8_t> bytes{1, 2};
  bse::BinaryReader reader(bytes);
  auto value = reader.ReadU32();
  BSE_EXPECT_TRUE(!value.ok());
  BSE_EXPECT_EQ(value.status().code(), bse::ErrorCode::kUnexpectedEof);
}

}  // namespace

int main() {
  RoundTripsLittleEndianNumbers();
  RejectsTruncatedReads();
  return 0;
}
