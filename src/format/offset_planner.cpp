#include "bse/format/offset_planner.hpp"
namespace bse::format_ext {
std::vector<PlannedSection> planOffsets(const std::vector<std::pair<format::SectionKind,std::uint64_t>>& sections,std::uint64_t start,std::uint64_t alignment){ std::vector<PlannedSection> out; auto cursor=start; for(auto& s:sections){ cursor=format::alignTo(cursor,alignment); out.push_back({s.first,cursor,s.second}); cursor+=s.second; } return out; }
std::uint64_t totalFileSize(const std::vector<PlannedSection>& plan){ std::uint64_t end=0; for(auto& p:plan) end=std::max(end,p.offset+p.size); return end; }
}
