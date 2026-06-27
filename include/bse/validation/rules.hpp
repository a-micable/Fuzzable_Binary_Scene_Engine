#pragma once
#include "bse/binary_scene_engine.hpp"
namespace bse::validation_ext {
ErrorList validateMeshTopology(const Scene& scene);
ErrorList validateMaterialTextures(const Scene& scene);
ErrorList validateAnimationTargets(const Scene& scene);
}
