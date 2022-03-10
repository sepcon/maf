#pragma once

#include <tuple>

#include "TypeTraits.h"

namespace maf {
namespace nstl {

struct tupe_for_each_interrupt {};

template <class F, class Tuple, std::size_t... Is>
void tuple_for_each(Tuple&& tuple, F func, std::index_sequence<Is...>) {
  using namespace std;
  using expander = int[];
  (void)expander{0, ((void)func(get<Is>(tuple)), 0)...};
}

template <class F, class Tuple>
void tuple_for_each(Tuple&& tuple, F func) {
  tuple_for_each(
      tuple, func,
      std::make_index_sequence<std::tuple_size_v<std::decay_t<Tuple>>>());
}

}  // namespace nstl
}  // namespace maf
