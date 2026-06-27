#include "bse/math/interpolation.hpp"
#include <cmath>
namespace bse::math {
float clamp01(float v){ return v<0.0f?0.0f:(v>1.0f?1.0f:v); }
float lerp(float a,float b,float t){ t=clamp01(t); return a+(b-a)*t; }
Vec3 lerp(const Vec3& a,const Vec3& b,float t){ return {lerp(a.x,b.x,t),lerp(a.y,b.y,t),lerp(a.z,b.z,t)}; }
Vec4 normalizeQuat(const Vec4& q){ float len=std::sqrt(q.x*q.x+q.y*q.y+q.z*q.z+q.w*q.w); if(len<=0.0f) return {0,0,0,1}; return {q.x/len,q.y/len,q.z/len,q.w/len}; }
Vec4 nlerpQuat(const Vec4& a,const Vec4& b,float t){ return normalizeQuat({lerp(a.x,b.x,t),lerp(a.y,b.y,t),lerp(a.z,b.z,t),lerp(a.w,b.w,t)}); }
}
