#pragma once

#include <maf/utils/cppextension/TypeTraits.h>
namespace maf {
namespace srz {

template<class TupleLike>
struct TupleLikeBase
{
    static constexpr bool isTupleLike() { return nstl::is_tuple_v<typename TupleLike::data_type>; }

private:
    using Impl = TupleLike;
};

template <typename T>
struct is_tuple_like
{
private:
    template<typename Type, std::enable_if_t<std::is_base_of_v<TupleLikeBase<Type>, Type>, bool> = true>
    static constexpr bool f_is_tuple_like() { return Type::isTupleLike(); }

    template<typename Type, std::enable_if_t<!std::is_base_of_v<TupleLikeBase<Type>, Type>, bool> = true>
    static constexpr bool f_is_tuple_like() { return false; }
public:
    static constexpr bool value = f_is_tuple_like<T>();
};

template <typename T>
static constexpr bool is_tuple_like_v = is_tuple_like<T>::value;

}
}
