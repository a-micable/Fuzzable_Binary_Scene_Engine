#include "bse/material/parameter_block.hpp"
namespace bse::material {
ParameterBlock parametersFor(const Material& m){ return {m.baseColor(),m.metallic(),m.roughness(),m.textureForSlot("baseColor")}; }
bool isOpaque(const ParameterBlock& b){ return b.baseColor.w>=0.999f; }
}
