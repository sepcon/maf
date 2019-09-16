#pragma once

#include <tuple>

namespace maf {
namespace nstl {

template<class F, class...Ts, std::size_t...Is>
void tuple_for_each(const std::tuple<Ts...> & tuple, F func, std::index_sequence<Is...>){
    using expander = int[];
    (void)expander { 0, ((void)func(std::get<Is>(tuple)), 0)... };
}

template<class F, class...Ts>
void tuple_for_each(const std::tuple<Ts...> & tuple, F func){
    tuple_for_each(tuple, func, std::make_index_sequence<sizeof...(Ts)>());
}

template<class F, class...Ts, std::size_t...Is>
void tuple_for_each(std::tuple<Ts...> & tuple, F func, std::index_sequence<Is...>){
    using expander = int[];
    (void)expander { 0, ((void)func(std::get<Is>(tuple)), 0)... };
}

template<class F, class...Ts>
void tuple_for_each(std::tuple<Ts...> & tuple, F func){
    tuple_for_each(tuple, func, std::make_index_sequence<sizeof...(Ts)>());
}

}
}
