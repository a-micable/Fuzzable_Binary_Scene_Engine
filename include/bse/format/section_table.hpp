#pragma once
#include "bse/binary_scene_engine.hpp"
#include <vector>
namespace bse::format_ext {
bool hasSection(const std::vector<format::SectionRecord>& records, format::SectionKind kind);
const format::SectionRecord* findSection(const std::vector<format::SectionRecord>& records, format::SectionKind kind);
std::vector<format::SectionRecord> sortedByOffset(std::vector<format::SectionRecord> records);
bool sectionsOverlap(const format::SectionRecord& a, const format::SectionRecord& b);
}
