#pragma once
#include "bse/binary_scene_engine.hpp"
#include <unordered_map>
namespace bse::skeleton {
std::unordered_map<std::string,ObjectId> buildBoneNameMap(const Skeleton& skeleton);
std::vector<ObjectId> boneChainToRoot(const Skeleton& skeleton, ObjectId boneId);
}
