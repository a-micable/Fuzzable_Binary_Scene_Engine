#include "bse/math/aabb.hpp"
#include <algorithm>
namespace bse::math {
Aabb emptyAabb(){ return {{0,0,0},{0,0,0},true}; }
Aabb includePoint(Aabb b,const Vec3& p){ if(b.empty){b.min=p;b.max=p;b.empty=false;return b;} b.min.x=std::min(b.min.x,p.x); b.min.y=std::min(b.min.y,p.y); b.min.z=std::min(b.min.z,p.z); b.max.x=std::max(b.max.x,p.x); b.max.y=std::max(b.max.y,p.y); b.max.z=std::max(b.max.z,p.z); return b; }
Aabb includeMesh(const Mesh& m){ Aabb b=emptyAabb(); for(const auto& v:m.vertices()) b=includePoint(b,v.position); return b; }
Vec3 center(const Aabb& b){ return b.empty?Vec3{}:Vec3{(b.min.x+b.max.x)*0.5f,(b.min.y+b.max.y)*0.5f,(b.min.z+b.max.z)*0.5f}; }
Vec3 extent(const Aabb& b){ return b.empty?Vec3{}:Vec3{b.max.x-b.min.x,b.max.y-b.min.y,b.max.z-b.min.z}; }
bool contains(const Aabb& b,const Vec3& p){ return !b.empty&&p.x>=b.min.x&&p.y>=b.min.y&&p.z>=b.min.z&&p.x<=b.max.x&&p.y<=b.max.y&&p.z<=b.max.z; }
}
