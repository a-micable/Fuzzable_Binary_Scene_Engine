#include "bse/binary_scene_engine.hpp"
#include <cstdint>
#include <vector>
extern "C" int LLVMFuzzerTestOneInput(const std::uint8_t* data, std::size_t size){ auto scene=bse::parser::SceneParser().parse(data,size); if(scene){ scene.value().traverse([](const bse::SceneNode&, const bse::Mat4&){}); } return 0; }
#ifndef BSE_LIBFUZZER
int main(){return 0;}
#endif
