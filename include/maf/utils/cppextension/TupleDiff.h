#pragma once

#include <bitset>
#include <tuple>

namespace maf {
namespace util {

template<class Copmare, class Tuple, std::size_t...Is>
void tuple_diff_impl(Tuple & tuple1,  Tuple& tuple2, Copmare compare, std::index_sequence<Is...>){
    using namespace std;
    using expander = int[];
    (void)expander { 0, ((void)compare(get<Is>(tuple1), get<Is>(tuple2), Is), 0)... };
}

template<class Tuple, class Compare>
void tuple_diff_impl(Tuple & tuple1,  Tuple& tuple2, Compare compare){
    tuple_diff_impl(tuple1, tuple2, compare, std::make_index_sequence<std::tuple_size_v<Tuple>>());
}

template <typename = void>
struct diff
{
    template<typename T>
    bool operator()(const T& t1, const T& t2)
    {
        return t1 != t2;
    }
};

template <class Tuple, class Compare = diff<>>
static std::bitset<std::tuple_size_v<Tuple>> tuple_diff(Tuple& t1, Tuple& t2)
{
    std::bitset<std::tuple_size_v<Tuple>> diffbits;
    tuple_diff_impl(t1, t2, [&diffbits](const auto& e1, const auto& e2, size_t idx) {
        if(Compare{}(e1, e2))
        {
            diffbits.set(idx);
        }
    });
    return diffbits;
}


} // util
} // maf
