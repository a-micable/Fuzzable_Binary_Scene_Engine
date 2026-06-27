#include "bse/skeleton/bone_map.hpp"
namespace bse::skeleton {
std::unordered_map<std::string,ObjectId> buildBoneNameMap(const Skeleton& s){ std::unordered_map<std::string,ObjectId> out; for(auto& b:s.bones()) out[b.name]=b.id; return out; }
std::vector<ObjectId> boneChainToRoot(const Skeleton& s,ObjectId id){ std::vector<ObjectId> out; for(auto* b=s.findBone(id); b; b=isValidId(b->parentId)?s.findBone(b->parentId):nullptr){ out.push_back(b->id); if(!isValidId(b->parentId)) break; } return out; }
}
