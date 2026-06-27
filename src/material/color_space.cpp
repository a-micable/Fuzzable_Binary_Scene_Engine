#include "bse/material/color_space.hpp"
#include <cmath>
namespace bse::material {
float srgbToLinear(float v){ return v<=0.04045f?v/12.92f:std::pow((v+0.055f)/1.055f,2.4f); }
float linearToSrgb(float v){ return v<=0.0031308f?v*12.92f:1.055f*std::pow(v,1.0f/2.4f)-0.055f; }
Vec4 srgbToLinear(Vec4 c){ return {srgbToLinear(c.x),srgbToLinear(c.y),srgbToLinear(c.z),c.w}; }
Vec4 linearToSrgb(Vec4 c){ return {linearToSrgb(c.x),linearToSrgb(c.y),linearToSrgb(c.z),c.w}; }
}
