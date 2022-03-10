#pragma once

#include <variant>

#include "Dumper.h"

namespace std {
template <class OStream, typename... _Args>
void _dump(OStream &ds, const std::variant<_Args...> &val,
           int indentLevel = 0) {
  std::visit(
      [&](auto &&v) {
        using namespace maf::srz;
        dump(ds, v, indentLevel);
      },
      val);
}

}  // namespace std
