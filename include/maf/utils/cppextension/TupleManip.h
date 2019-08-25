#pragma once

#include <tuple>

namespace maf {
namespace nstl {


template <int Max>
struct TuppleManipulator
{
    template <class Tuple, typename FunctionOneParam>
    static void apply(Tuple tuple_, FunctionOneParam f)
    {
        TuppleManipulator<Max - 1>::template apply<Tuple, FunctionOneParam>(tuple_, f);
        f(std::get<Max - 1>(tuple_));
    }
    template <class Tuple1, class Tuple2, typename FunctionTwoParams,
             std::enable_if_t<std::tuple_size_v<Tuple1> == std::tuple_size_v<Tuple2>, bool> = true>
    static void apply(Tuple1 tuple1_, Tuple2 tuple2_, FunctionTwoParams f)
    {
        TuppleManipulator<Max - 1>::template apply<Tuple1, Tuple2, FunctionTwoParams>(tuple1_, tuple2_, f);
        f(std::get<Max - 1>(tuple1_), std::get<Max - 1>(tuple2_));
    }
};

template <>
struct TuppleManipulator<0>
{
    template <class Tuple, typename Function>
    static void apply(Tuple, Function)
    {
    }
    template <class Tuple1, class Tuple2, typename FunctionTwoParams,
             std::enable_if_t<std::tuple_size_v<Tuple1> == std::tuple_size_v<Tuple2>, bool> = true>
    static void apply(Tuple1, Tuple2, FunctionTwoParams)
    {
    }
};

template<class Tuple, typename Function>
void tuple_for_each(const Tuple& t, Function f)
{
    using ConstRefTuple = typename std::add_const<Tuple>::type &;
    TuppleManipulator<std::tuple_size<Tuple>::value>::template apply<ConstRefTuple, Function>(t, f);
}

template<class Tuple, typename Function>
void tuple_for_each(Tuple& t, Function f)
{
    using RefTuple = Tuple & ;
    TuppleManipulator<std::tuple_size<Tuple>::value>::template apply<RefTuple, Function>(t, f);
}

template<class Tuple1, class Tuple2, typename Function>
void tuple_for_each2(const Tuple1& t, const Tuple2& t2, Function f)
{
    TuppleManipulator<std::tuple_size<Tuple1>::value>::template apply<Tuple1, Tuple2, Function>(t, t2, f);
}

template<class Tuple1, class Tuple2, typename FunctionTwoParams>
void tuple_for_each2(Tuple1& t1, const Tuple2& t2, FunctionTwoParams f)
{
    TuppleManipulator<std::tuple_size_v<Tuple1>>::template apply<Tuple1, Tuple2, FunctionTwoParams>(t1, t2, f);
}


}
}
