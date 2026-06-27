#pragma once
#include "bse/binary_scene_engine.hpp"
namespace bse::mesh {
bool allIndicesInRange(const Mesh& mesh);
std::uint32_t highestIndex(const Mesh& mesh);
std::vector<std::uint32_t> compactReferencedVertices(const Mesh& mesh);
}
