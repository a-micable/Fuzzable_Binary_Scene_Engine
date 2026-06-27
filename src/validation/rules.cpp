#include "bse/validation/rules.hpp"
namespace bse::validation_ext {
ErrorList validateMeshTopology(const Scene& s){ ErrorList e; for(auto& m:s.meshes()) if(m.indices().size()%3!=0) e.add(ErrorCode::ValidationError,"mesh index count is not divisible by three",m.name()); return e; }
ErrorList validateMaterialTextures(const Scene& s){ ErrorList e; for(auto& m:s.materials()) for(auto& slot:m.textureSlots()) if(isValidId(slot.second)&&!s.findTexture(slot.second)) e.add(ErrorCode::MissingReference,"material references missing texture",m.name()+":"+slot.first); return e; }
ErrorList validateAnimationTargets(const Scene& s){ ErrorList e; for(auto& a:s.animations()) for(auto& ch:a.channels()) if(isValidId(ch.targetNode())&&!s.findNode(ch.targetNode())) e.add(ErrorCode::MissingReference,"animation targets missing node",a.name()); return e; }
}
