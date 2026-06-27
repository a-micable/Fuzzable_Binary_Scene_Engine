#include "bse/animation/evaluator.hpp"
namespace bse::animation {
std::unordered_map<ObjectId,Transform> sampleClip(const AnimationClip& clip,float time){ std::unordered_map<ObjectId,Transform> out; for(const auto& ch:clip.channels()) out[ch.targetNode()]=ch.sample(time); return out; }
void applySampledTransforms(Scene& scene,const std::unordered_map<ObjectId,Transform>& t){ for(auto& item:t) if(auto* n=scene.findNode(item.first)) n->localTransform()=item.second; }
}
