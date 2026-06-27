#include "bse/scene/scene_query.hpp"
namespace bse::scene {
std::vector<const SceneNode*> nodesUsingMesh(const Scene& s,ObjectId id){ std::vector<const SceneNode*> out; for(auto& n:s.nodes()) if(n.meshId()==id) out.push_back(&n); return out; }
std::vector<const Mesh*> meshesUsingMaterial(const Scene& s,ObjectId id){ std::vector<const Mesh*> out; for(auto& m:s.meshes()) if(m.materialId()==id) out.push_back(&m); return out; }
const SceneNode* firstCameraNode(const Scene& s){ for(auto& n:s.nodes()) if(isValidId(n.cameraId())) return &n; return nullptr; }
}
