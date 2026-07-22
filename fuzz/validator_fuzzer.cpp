#include "bse/parser.hpp"
#include "bse/validator.hpp"

#include "fuzz_support.hpp"

#include <cstddef>
#include <cstdint>

extern "C" int LLVMFuzzerTestOneInput(const std::uint8_t* data, std::size_t size) {
  auto parsed_input = bse::ParseScene(data, size);
  if (parsed_input.ok()) {
    bse::DiagnosticSink parsed_diagnostics;
    auto parsed_status = bse::ValidateScene(parsed_input.value(), &parsed_diagnostics);
    (void)parsed_status;
  }

  auto scene = bse_fuzz::SceneFromBytes(data, size);
  if (size > 1 && (data[1] & 1U) != 0U) {
    scene.nodes.front().mesh = 9999;
  }
  bse::DiagnosticSink diagnostics;
  auto status = bse::ValidateScene(scene, &diagnostics);
  (void)status;
  return 0;
}
