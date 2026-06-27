#pragma once
#include "bse/binary_scene_engine.hpp"
#include <vector>
namespace bse::format_ext {
struct PlannedSection { format::SectionKind kind; std::uint64_t offset; std::uint64_t size; };
std::vector<PlannedSection> planOffsets(const std::vector<std::pair<format::SectionKind,std::uint64_t>>& sections, std::uint64_t start, std::uint64_t alignment);
std::uint64_t totalFileSize(const std::vector<PlannedSection>& plan);
}
