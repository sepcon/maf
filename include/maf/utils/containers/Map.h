#pragma once

#include <map>

namespace maf {
namespace util {

template <typename Key, typename Value>
Value get(const std::map<Key, Value> &m, const Key &key) {
  if (auto itVal = m.find(key); itVal != m.end()) {
    return itVal->second;
  }
  return {};
}
} // namespace util
} // namespace maf
