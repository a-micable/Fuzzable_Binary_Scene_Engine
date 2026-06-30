#include "bse/parser.hpp"
#include "bse/serializer.hpp"

#include "fuzz_support.hpp"

#include <cstddef>
#include <cstdint>

extern "C" int LLVMFuzzerTestOneInput(const std::uint8_t* data, std::size_t size) {
  auto parsed_input = bse::ParseScene(data, size);
  if (parsed_input.ok()) {
    auto reserialized = bse::SerializeScene(parsed_input.value());
    (void)reserialized;
  }

  auto bytes = bse::SerializeScene(bse_fuzz::SceneFromBytes(data, size));
  if (bytes.ok()) {
    auto parsed = bse::ParseScene(bytes.value());
    (void)parsed;
  }
  return 0;
}
