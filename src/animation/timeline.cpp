#include "bse/animation/timeline.hpp"
#include <algorithm>
#include <cmath>
namespace bse::animation {
TimelineRange rangeOf(const AnimationClip& clip){ TimelineRange r; for(const auto& ch:clip.channels()){ for(auto& k:ch.translations()){ if(r.empty){r.start=r.end=k.time;r.empty=false;} r.start=std::min(r.start,k.time); r.end=std::max(r.end,k.time);} } return r; }
float wrapTime(float t,float d){ if(d<=0.0f) return 0.0f; float x=std::fmod(t,d); return x<0?x+d:x; }
}
