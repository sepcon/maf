#pragma once

#include <maf/utils/cppextension/TypeTraits.h>
namespace maf {
namespace srz {

template<class TupleLike>
struct TuplizableTypeBase
{
    static constexpr bool isTupleLike() { return nstl::is_tuple_v<typename TupleLike::data_type>; }

private:
    using Impl = TupleLike;
};

template <typename T>
struct is_tuplizable_type
{
private:
    template<typename Type, std::enable_if_t<std::is_base_of_v<TuplizableTypeBase<Type>, Type>, bool> = true>
    static constexpr bool f_is_tuplizable_type() { return Type::isTupleLike(); }

    template<typename Type, std::enable_if_t<!std::is_base_of_v<TuplizableTypeBase<Type>, Type>, bool> = true>
    static constexpr bool f_is_tuplizable_type() { return false; }
public:
    static constexpr bool value = f_is_tuplizable_type<T>();
};

template <typename T>
static constexpr bool is_tuplizable_type_v = is_tuplizable_type<T>::value;

}
}
