#include "bse/resource/cache_policy.hpp"
namespace bse::resource_ext { bool shouldEvict(std::size_t live,std::size_t bytes,const CachePolicy& p){ return live>p.maxEntries || bytes>p.softBytes; } }
