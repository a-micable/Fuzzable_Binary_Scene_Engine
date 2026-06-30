#pragma once

#include "bse/diagnostics.hpp"
#include "bse/scene.hpp"
#include "bse/status.hpp"

#include <cstddef>

namespace bse {

struct ValidationLimits {
  std::size_t max_nodes = 1'000'000;
  std::size_t max_vertices = 64'000'000;
  std::size_t max_indices = 192'000'000;
};

Status ValidateScene(const Scene& scene, DiagnosticSink* diagnostics = nullptr,
                     const ValidationLimits& limits = ValidationLimits{});

}  // namespace bse
