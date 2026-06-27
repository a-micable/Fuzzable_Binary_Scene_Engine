#include "bse/compression/entropy.hpp"
#include <array>
#include <cmath>
namespace bse::compression_ext {
double byteEntropy(const std::vector<std::uint8_t>& in){ if(in.empty()) return 0.0; std::array<std::size_t,256> freq{}; for(auto b:in) ++freq[b]; double e=0.0; for(auto f:freq) if(f){ double p=static_cast<double>(f)/static_cast<double>(in.size()); e-=p*std::log2(p);} return e; }
bool looksCompressible(const std::vector<std::uint8_t>& in){ return in.size()>64 && byteEntropy(in)<6.5; }
}
