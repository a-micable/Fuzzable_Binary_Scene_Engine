#include "bse/resource_cache.hpp"

#include "test_support.hpp"

namespace {

void EvictsLeastRecentlyUsedResource() {
  bse::ResourceCache cache(6);
  BSE_EXPECT_TRUE(cache.Put("a", {1, 2, 3}));
  BSE_EXPECT_TRUE(cache.Put("b", {4, 5, 6}));
  BSE_EXPECT_TRUE(cache.Get("a") != nullptr);
  BSE_EXPECT_TRUE(cache.Put("c", {7, 8, 9}));

  BSE_EXPECT_TRUE(cache.Contains("a"));
  BSE_EXPECT_TRUE(!cache.Contains("b"));
  BSE_EXPECT_TRUE(cache.Contains("c"));
  BSE_EXPECT_EQ(cache.size_bytes(), 6U);
}

void RejectsResourcesLargerThanCapacity() {
  bse::ResourceCache cache(2);
  BSE_EXPECT_TRUE(!cache.Put("too-large", {1, 2, 3}));
  BSE_EXPECT_TRUE(!cache.Contains("too-large"));
  BSE_EXPECT_EQ(cache.size_bytes(), 0U);
}

}  // namespace

int main() {
  EvictsLeastRecentlyUsedResource();
  RejectsResourcesLargerThanCapacity();
  return 0;
}
