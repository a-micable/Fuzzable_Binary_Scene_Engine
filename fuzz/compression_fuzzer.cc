#include "bse/binary_scene_engine.hpp"
#include <cstdint>
#include <vector>
extern "C" int LLVMFuzzerTestOneInput(const std::uint8_t* data, std::size_t size){ std::vector<std::uint8_t> input(data,data+size); auto c=bse::compression::compressRle(input); (void)bse::compression::decompressRle(c,input.size()); return 0; }
#ifndef BSE_LIBFUZZER
int main(){return 0;}
#endif
