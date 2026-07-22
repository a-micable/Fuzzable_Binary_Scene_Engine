#pragma once

#include "bse/scene.hpp"
#include "bse/status.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace bse {

Result<Scene> ParseScene(const std::uint8_t* data, std::size_t size);
Result<Scene> ParseScene(const std::vector<std::uint8_t>& data);

}  // namespace bse
