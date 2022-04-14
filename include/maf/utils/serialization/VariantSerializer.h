#pragma once

#include <maf/utils/cppextension/Macros.h>

#include <tuple>
#include <variant>

#include "Serializer.h"

namespace std {

template <class OStream, class... _Args>
void _serialize(OStream &os, const variant<_Args...> &v) {
  static_assert(sizeof...(_Args) <= 20,
                "Not support variant with more than 20 types");
  visit(
      [&](auto const &e) {
        using namespace maf::srz;
        const SizeType idx = v.index();
        serialize(os, idx);
        serialize(os, e);
      },
      v);
}

template <class IStream, class... _Args>
bool _deserialize(IStream &is, variant<_Args...> &v) {
  using namespace maf::srz;

#define _maf_vs_dispatch_deserializer(idx) switch (idx)
#define _maf_vs_deserialize_if_match(idx)                \
  case idx: {                                            \
    if constexpr (idx < sizeof...(_Args)) {              \
      std::tuple_element_t<idx, std::tuple<_Args...>> e; \
      if (deserialize(is, e)) {                          \
        v = std::move(e);                                \
        return true;                                     \
      } else {                                           \
        return false;                                    \
      }                                                  \
    } else {                                             \
      return false;                                      \
    }                                                    \
  } break
#define _maf_vs_default_deserializer() \
  default:                             \
    return false

  SizeType idx = 0;
  if (deserialize(is, idx) && idx >= 0 && idx < sizeof...(_Args)) {
    _maf_vs_dispatch_deserializer(idx) {
      _maf_vs_deserialize_if_match(0);
      _maf_vs_deserialize_if_match(1);
      _maf_vs_deserialize_if_match(2);
      _maf_vs_deserialize_if_match(3);
      _maf_vs_deserialize_if_match(4);
      _maf_vs_deserialize_if_match(5);
      _maf_vs_deserialize_if_match(6);
      _maf_vs_deserialize_if_match(7);
      _maf_vs_deserialize_if_match(8);
      _maf_vs_deserialize_if_match(9);
      _maf_vs_deserialize_if_match(10);
      _maf_vs_deserialize_if_match(11);
      _maf_vs_deserialize_if_match(12);
      _maf_vs_deserialize_if_match(13);
      _maf_vs_deserialize_if_match(14);
      _maf_vs_deserialize_if_match(15);
      _maf_vs_deserialize_if_match(16);
      _maf_vs_deserialize_if_match(17);
      _maf_vs_deserialize_if_match(18);
      _maf_vs_deserialize_if_match(19);
      _maf_vs_default_deserializer();
    }
  }
#undef _maf_vs_dispatch_deserializer
#undef _maf_vs_deserialize_if_match
#undef _maf_vs_default_deserializer
  return false;
}

}  // namespace std
