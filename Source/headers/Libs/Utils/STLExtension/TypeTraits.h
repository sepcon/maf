#ifndef TYPETRAITS_H
#define TYPETRAITS_H

#include <type_traits>
#include <iterator>

namespace thaf {
namespace stl {

template <typename T>
struct type_name_traits;

#define REGISTER_TYPE_NAME(T) namespace thaf { namespace stl { template<> struct type_name_traits<T> {\
    inline static constexpr const char* const name = #T; \
}; } }

template <typename T>
struct is_number_type
{
    inline static constexpr bool value = std::is_enum_v<T> || std::is_integral_v<T> || std::is_floating_point_v<T>;
};

template <typename T>
inline static constexpr bool is_number_type_v = is_number_type<T>::value;


template <typename T, template <typename...> class Template>
struct is_specialization_of : std::integral_constant<bool, false> {};

template <template <typename...> class Template, typename... Args>
struct is_specialization_of<Template<Args...>, Template> : std::integral_constant<bool,true> {};

template <typename>
struct is_tuple: std::false_type {};

template <typename ...T>
struct is_tuple<std::tuple<T...>>: std::true_type {};

template <typename T>
inline static constexpr bool is_tuple_v = is_tuple<T>::value;

template <typename Container>
using iterator_category_of = typename std::iterator_traits<typename Container::iterator>::iterator_category;

template <typename Container>
struct has_random_access_iterator : public std::is_base_of<std::random_access_iterator_tag, iterator_category_of<Container>>{};

template <typename Container>
inline static constexpr bool has_random_access_iterator_v = has_random_access_iterator<Container>::value;


} // sr
} // thaf
#endif // TYPETRAITS_H
