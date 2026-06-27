
#include "bse/binary_scene_engine.hpp"
#include <fstream>
#include <iostream>
#include <iterator>
int main(int argc, char** argv){ if(argc!=3){std::cerr<<"usage: bse_roundtrip <in> <out>\n"; return 2;} std::ifstream in(argv[1],std::ios::binary); std::vector<std::uint8_t> bytes((std::istreambuf_iterator<char>(in)),{}); auto scene=bse::serialization::SceneDeserializer().deserialize(bytes); if(!scene){std::cerr<<scene.error().message<<"\n"; return 1;} auto out=bse::serialization::SceneSerializer().serialize(scene.value()); if(!out){std::cerr<<out.error().message<<"\n"; return 1;} std::ofstream file(argv[2],std::ios::binary); file.write(reinterpret_cast<const char*>(out.value().data()), static_cast<std::streamsize>(out.value().size())); }
