#pragma once

#include <maf/threading/Lockable.h>
#include <map>

namespace maf {
namespace util {

#define maf_map2d_template                                                     \
  template <typename Key1, typename Key2, typename Value>

maf_map2d_template using Map2D = std::map<Key1, std::map<Key2, Value>>;

maf_map2d_template Value find(const Map2D<Key1, Key2, Value> &map2d,
                              const Key1 &k1, const Key2 &k2) {
  if (auto itValueMap = map2d.find(k1); itValueMap != map2d.end()) {
    auto &valueMap = itValueMap->second;
    if (auto itValue = valueMap.find(k2); itValue != valueMap.end()) {
      return itValue->second;
    }
  }
  return Value{};
}

#undef maf_map2d_template

} // namespace util
} // namespace maf
