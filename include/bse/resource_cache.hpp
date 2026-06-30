#pragma once

#include <cstddef>
#include <cstdint>
#include <list>
#include <string>
#include <unordered_map>
#include <vector>

namespace bse {

struct CachedResource {
  std::string key;
  std::vector<std::uint8_t> bytes;
};

class ResourceCache {
 public:
  explicit ResourceCache(std::size_t capacity_bytes);

  bool Put(std::string key, std::vector<std::uint8_t> bytes);
  const CachedResource* Get(const std::string& key);
  bool Contains(const std::string& key) const;
  std::size_t size_bytes() const { return size_bytes_; }
  std::size_t capacity_bytes() const { return capacity_bytes_; }
  void Clear();

 private:
  using EntryList = std::list<CachedResource>;
  using EntryIt = EntryList::iterator;

  void Touch(EntryIt it);
  void EvictUntilWithinCapacity();

  std::size_t capacity_bytes_ = 0;
  std::size_t size_bytes_ = 0;
  EntryList entries_;
  std::unordered_map<std::string, EntryIt> index_;
};

}  // namespace bse
