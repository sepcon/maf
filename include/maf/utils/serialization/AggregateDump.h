#pragma once

#include <maf/utils/cppextension/Aggregate.h>

#include "Dumper.h"

namespace maf {
namespace srz {

using namespace nstl;
namespace details {

template <class OStream, typename Struct>
struct DumperSFINAE<OStream, Struct,
                    std::enable_if_t<std::is_aggregate_v<Struct> &&
                                         !has_cas_tuple_method<Struct>::value,
                                     void>> {
  static void write(OStream &ds, const Struct &value,
                    int indentLevel) noexcept {
    using namespace maf::srz;
    dump(ds, nstl::tuple_view(value), indentLevel);
  }
};

}  // namespace details
}  // namespace srz
}  // namespace maf
