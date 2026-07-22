#pragma once

#include "bse/scene.hpp"
#include "bse/status.hpp"

#include <cstdint>
#include <vector>

namespace bse {

Result<std::vector<std::uint8_t>> SerializeScene(const Scene& scene);

}  // namespace bse
