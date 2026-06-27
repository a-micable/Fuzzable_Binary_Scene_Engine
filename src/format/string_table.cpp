#include "bse/format/string_table.hpp"
#include "bse/binary_scene_engine.hpp"
namespace bse::format_ext {
std::uint32_t StringTableBuilder::intern(const std::string& v){ auto f=index_.find(v); if(f!=index_.end()) return f->second; auto id=static_cast<std::uint32_t>(values_.size()); index_[v]=id; values_.push_back(v); return id; }
const std::string& StringTableBuilder::at(std::uint32_t i) const{ return values_.at(i); }
std::vector<std::string> StringTableBuilder::ordered() const{ return values_; }
std::vector<std::uint8_t> StringTableBuilder::serialize() const{ BinaryWriter w; w.writePod<std::uint32_t>(values_.size()); for(const auto& v:values_){ auto n=static_cast<std::uint16_t>(v.size()); w.writePod(n); w.writeString(v); } return w.bytes(); }
}
