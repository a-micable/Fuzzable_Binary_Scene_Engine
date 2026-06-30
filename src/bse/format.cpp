#include "bse/format.hpp"

namespace bse {

const char* SectionName(SectionType type) {
  switch (type) {
    case SectionType::kMetadata:
      return "metadata";
    case SectionType::kNodes:
      return "nodes";
    case SectionType::kMeshes:
      return "meshes";
    case SectionType::kMaterials:
      return "materials";
    case SectionType::kTextures:
      return "textures";
    case SectionType::kAnimations:
      return "animations";
    case SectionType::kSkeletons:
      return "skeletons";
    case SectionType::kCameras:
      return "cameras";
    case SectionType::kLights:
      return "lights";
  }
  return "unknown";
}

}  // namespace bse
