#include "bse/resource_cache.hpp"

#include <utility>

namespace bse {

ResourceCache::ResourceCache(std::size_t capacity_bytes) : capacity_bytes_(capacity_bytes) {}

bool ResourceCache::Put(std::string key, std::vector<std::uint8_t> bytes) {
  if (bytes.size() > capacity_bytes_) {
    return false;
  }
  auto existing = index_.find(key);
  if (existing != index_.end()) {
    size_bytes_ -= existing->second->bytes.size();
    entries_.erase(existing->second);
    index_.erase(existing);
  }
  size_bytes_ += bytes.size();
  entries_.push_front(CachedResource{std::move(key), std::move(bytes)});
  index_[entries_.front().key] = entries_.begin();
  EvictUntilWithinCapacity();
  return true;
}

const CachedResource* ResourceCache::Get(const std::string& key) {
  auto it = index_.find(key);
  if (it == index_.end()) {
    return nullptr;
  }
  Touch(it->second);
  return &entries_.front();
}

bool ResourceCache::Contains(const std::string& key) const {
  return index_.find(key) != index_.end();
}

void ResourceCache::Clear() {
  entries_.clear();
  index_.clear();
  size_bytes_ = 0;
}

void ResourceCache::Touch(EntryIt it) {
  entries_.splice(entries_.begin(), entries_, it);
}

void ResourceCache::EvictUntilWithinCapacity() {
  while (size_bytes_ > capacity_bytes_ && !entries_.empty()) {
    const auto& entry = entries_.back();
    size_bytes_ -= entry.bytes.size();
    index_.erase(entry.key);
    entries_.pop_back();
  }
}

}  // namespace bse
