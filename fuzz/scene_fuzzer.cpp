#include "bse/parser.hpp"

#include <cstddef>
#include <cstdint>

extern "C" int LLVMFuzzerTestOneInput(const std::uint8_t* data, std::size_t size) {
  auto scene = bse::ParseScene(data, size);
  (void)scene;
  return 0;
}
