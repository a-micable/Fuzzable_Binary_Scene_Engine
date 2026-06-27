#pragma once
#include "bse/binary_scene_engine.hpp"
#include <set>
namespace bse::mesh {
struct TopologyStats { std::size_t triangleCount=0; std::size_t degenerateTriangles=0; std::size_t uniqueEdges=0; };
TopologyStats analyzeTopology(const Mesh& mesh);
bool isDegenerate(const Vertex& a, const Vertex& b, const Vertex& c);
}
