#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>
namespace bse::format_ext {
class StringTableBuilder { public: std::uint32_t intern(const std::string& value); const std::string& at(std::uint32_t index) const; std::vector<std::string> ordered() const; std::vector<std::uint8_t> serialize() const; private: std::unordered_map<std::string,std::uint32_t> index_; std::vector<std::string> values_; };
}
