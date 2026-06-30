#include "bse/parser.hpp"
#include "bse/serializer.hpp"

#include "fuzz_support.hpp"

#include <cstddef>
#include <cstdint>

extern "C" int LLVMFuzzerTestOneInput(const std::uint8_t* data, std::size_t size) {
  auto parsed_input = bse::ParseScene(data, size);
  if (parsed_input.ok()) {
    auto bytes = bse::SerializeScene(parsed_input.value());
    (void)bytes;
  }

  auto scene = bse_fuzz::SceneFromBytes(data, size);
  if (size > 0 && (data[0] & 1U) != 0U) {
    scene.nodes.front().id = 0;
  }
  auto bytes = bse::SerializeScene(scene);
  (void)bytes;
  return 0;
}
