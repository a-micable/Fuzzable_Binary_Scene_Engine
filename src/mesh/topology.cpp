#include "bse/mesh/topology.hpp"
#include <cmath>
namespace bse::mesh {
bool isDegenerate(const Vertex& a,const Vertex& b,const Vertex& c){ float ux=b.position.x-a.position.x, uy=b.position.y-a.position.y, uz=b.position.z-a.position.z; float vx=c.position.x-a.position.x, vy=c.position.y-a.position.y, vz=c.position.z-a.position.z; float cx=uy*vz-uz*vy, cy=uz*vx-ux*vz, cz=ux*vy-uy*vx; return cx*cx+cy*cy+cz*cz<1e-10f; }
TopologyStats analyzeTopology(const Mesh& m){ TopologyStats s; std::set<std::pair<std::uint32_t,std::uint32_t>> edges; for(std::size_t i=0;i+2<m.indices().size();i+=3){ auto a=m.indices()[i],b=m.indices()[i+1],c=m.indices()[i+2]; if(a>=m.vertices().size()||b>=m.vertices().size()||c>=m.vertices().size()) continue; ++s.triangleCount; if(isDegenerate(m.vertices()[a],m.vertices()[b],m.vertices()[c])) ++s.degenerateTriangles; for(auto e:{std::pair{a,b},std::pair{b,c},std::pair{c,a}}){ if(e.first>e.second) std::swap(e.first,e.second); edges.insert(e);} } s.uniqueEdges=edges.size(); return s; }
}
