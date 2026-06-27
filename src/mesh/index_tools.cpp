#include "bse/mesh/index_tools.hpp"
#include <algorithm>
namespace bse::mesh {
bool allIndicesInRange(const Mesh& m){ for(auto i:m.indices()) if(i>=m.vertices().size()) return false; return true; }
std::uint32_t highestIndex(const Mesh& m){ return m.indices().empty()?0:*std::max_element(m.indices().begin(),m.indices().end()); }
std::vector<std::uint32_t> compactReferencedVertices(const Mesh& m){ std::vector<std::uint32_t> refs=m.indices(); std::sort(refs.begin(),refs.end()); refs.erase(std::unique(refs.begin(),refs.end()),refs.end()); return refs; }
}
