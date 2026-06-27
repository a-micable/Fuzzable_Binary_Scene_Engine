#include "bse/scene/hierarchy.hpp"
namespace bse::scene {
bool hasAncestor(const Scene& s,ObjectId node,ObjectId ancestor){ auto* n=s.findNode(node); while(n&&isValidId(n->parentId())){ if(n->parentId()==ancestor) return true; n=s.findNode(n->parentId()); } return false; }
std::vector<ObjectId> descendantsOf(const Scene& s,ObjectId root){ std::vector<ObjectId> out; if(auto* n=s.findNode(root)){ for(auto c:n->children()){ out.push_back(c); auto sub=descendantsOf(s,c); out.insert(out.end(),sub.begin(),sub.end()); }} return out; }
std::size_t depthOf(const Scene& s,ObjectId node){ std::size_t d=0; auto* n=s.findNode(node); while(n&&isValidId(n->parentId())){ ++d; n=s.findNode(n->parentId()); } return d; }
}
