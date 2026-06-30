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
  if (size >= 4) {
    bse::Animation animation;
    animation.id = 40;
    animation.name = "fuzz_animation";
    animation.duration_seconds = static_cast<float>(data[0] % 60U);
    bse::AnimationChannel channel;
    channel.target_node = 1;
    channel.keys.push_back({0.0F, {}});
    channel.keys.push_back({animation.duration_seconds, {}});
    animation.channels.push_back(channel);
    scene.animations.push_back(animation);
  }
  auto bytes = bse::SerializeScene(scene);
  if (bytes.ok()) {
    auto parsed = bse::ParseScene(bytes.value());
    (void)parsed;
  }
  return 0;
}
