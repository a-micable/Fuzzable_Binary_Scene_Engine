#pragma once
#include <cstddef>
namespace bse::resource_ext {
struct CachePolicy { std::size_t maxEntries=1024; std::size_t softBytes=64*1024*1024; bool evictUnusedFirst=true; };
bool shouldEvict(std::size_t liveEntries, std::size_t estimatedBytes, const CachePolicy& policy);
}
