
#include "bse/binary_scene_engine.hpp"
#include <fstream>
#include <iostream>
#include <iterator>
int main(int argc, char** argv){ if(argc!=2){std::cerr<<"usage: bse_dump <scene.bsen>\n"; return 2;} std::ifstream in(argv[1],std::ios::binary); std::vector<std::uint8_t> bytes((std::istreambuf_iterator<char>(in)),{}); auto scene=bse::parser::SceneParser().parse(bytes); if(!scene){std::cerr<<scene.error().message<<"\n"; return 1;} std::cout<<scene.value().name()<<": "<<scene.value().nodes().size()<<" nodes, "<<scene.value().meshes().size()<<" meshes\n"; }
