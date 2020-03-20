#pragma once

#include <tuple>

namespace maf {
namespace nstl {

template <class F, class Tuple, std::size_t... Is>
void tuple_for_each(Tuple &tuple, F func, std::index_sequence<Is...>) {
  using namespace std;
  using expander = int[];
  (void)expander{0, ((void)func(get<Is>(tuple)), 0)...};
}

template <class F, class Tuple> void tuple_for_each(Tuple &tuple, F func) {
  tuple_for_each(tuple, func,
                 std::make_index_sequence<std::tuple_size_v<Tuple>>());
}

} // namespace nstl
} // namespace maf
