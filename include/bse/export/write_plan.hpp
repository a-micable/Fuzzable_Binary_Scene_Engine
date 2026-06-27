#pragma once
#include "bse/binary_scene_engine.hpp"
namespace bse::exporter {
struct WritePlan { std::size_t meshBytes=0; std::size_t materialBytes=0; std::size_t nodeBytes=0; std::size_t totalBytes=0; };
WritePlan estimateWritePlan(const Scene& scene);
}
