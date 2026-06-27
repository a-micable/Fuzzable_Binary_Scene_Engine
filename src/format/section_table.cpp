#include "bse/format/section_table.hpp"
#include <algorithm>
namespace bse::format_ext {
bool hasSection(const std::vector<format::SectionRecord>& r,format::SectionKind k){ return findSection(r,k)!=nullptr; }
const format::SectionRecord* findSection(const std::vector<format::SectionRecord>& r,format::SectionKind k){ for(const auto& x:r) if(x.kind==static_cast<std::uint32_t>(k)) return &x; return nullptr; }
std::vector<format::SectionRecord> sortedByOffset(std::vector<format::SectionRecord> r){ std::sort(r.begin(),r.end(),[](const auto& a,const auto& b){return a.offset<b.offset;}); return r; }
bool sectionsOverlap(const format::SectionRecord& a,const format::SectionRecord& b){ return a.offset<b.offset+b.size && b.offset<a.offset+a.size; }
}
