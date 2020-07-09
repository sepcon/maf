#pragma once

#include <type_traits>

namespace maf {
namespace util {

template <typename T> void assign_ptr(T *dest, T src) {
  if (dest) {
    *dest = std::move(src);
  }
}

template <typename T> void assign_ptr(T *dest, T *src) {
  if (dest && src) {
    *dest = *src;
  }
}

template <typename T> void assign_ptr(T &dest, T *src) {
  if (src) {
    dest = *src;
  }
}

} // namespace util
} // namespace maf
