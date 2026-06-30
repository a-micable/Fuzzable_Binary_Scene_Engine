#include "bse/compression.hpp"

#include <limits>

namespace bse {

Result<std::vector<std::uint8_t>> CompressRle(const std::vector<std::uint8_t>& input) {
  std::vector<std::uint8_t> output;
  output.reserve(input.size());
  for (std::size_t i = 0; i < input.size();) {
    const std::uint8_t value = input[i];
    std::uint8_t run = 1;
    while (i + run < input.size() && input[i + run] == value &&
           run < std::numeric_limits<std::uint8_t>::max()) {
      ++run;
    }
    output.push_back(run);
    output.push_back(value);
    i += run;
  }
  return output;
}

Result<std::vector<std::uint8_t>> DecompressRle(const std::vector<std::uint8_t>& input) {
  if ((input.size() % 2U) != 0U) {
    return Status::Error(ErrorCode::kMalformedData, "RLE stream must contain count/value pairs");
  }
  std::vector<std::uint8_t> output;
  for (std::size_t i = 0; i < input.size(); i += 2U) {
    const std::uint8_t run = input[i];
    const std::uint8_t value = input[i + 1U];
    output.insert(output.end(), run, value);
  }
  return output;
}

}  // namespace bse
