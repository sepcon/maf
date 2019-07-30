#pragma once

#include <tuple>

namespace thaf {
namespace nstl {


template <int Max>
struct TuppleManipulator
{
	template <class Tuple, typename Function>
	static void apply(Tuple tuple_, Function f)
	{
		TuppleManipulator<Max - 1>::template apply<Tuple, Function>(tuple_, f);
		f(std::get<Max - 1>(tuple_));
	}
};

template <>
struct TuppleManipulator<0>
{
	template <class Tuple, typename Function>
    static void apply(Tuple, Function)
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

}
}
