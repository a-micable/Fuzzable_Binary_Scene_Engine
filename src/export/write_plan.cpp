#include "bse/export/write_plan.hpp"
namespace bse::exporter {
WritePlan estimateWritePlan(const Scene& s){ WritePlan p; for(auto& m:s.meshes()) p.meshBytes+=m.vertices().size()*sizeof(Vertex)+m.indices().size()*sizeof(std::uint32_t)+m.name().size(); for(auto& m:s.materials()) p.materialBytes+=sizeof(Vec4)+sizeof(float)*2+m.name().size(); for(auto& n:s.nodes()) p.nodeBytes+=sizeof(Transform)+n.name().size(); p.totalBytes=p.meshBytes+p.materialBytes+p.nodeBytes+sizeof(format::FileHeader); return p; }
}
