#ifndef TYPETRAITS_H
#define TYPETRAITS_H

#include <iterator>
#include <memory>
#include <type_traits>

namespace maf {
namespace nstl {

template <typename T> struct type_name_traits;

#define REGISTER_TYPE_NAME(T)                                                  \
  namespace maf {                                                              \
  namespace nstl {                                                             \
  template <> struct type_name_traits<T> {                                     \
    static constexpr const char *const name = #T;                              \
  };                                                                           \
  }                                                                            \
  }

/// Remove constness and reference from type T
/// e.g: pure_type<const int&>::type == int
template <typename T> struct pure_type {
  using type = typename std::remove_const_t<std::remove_reference_t<T>>;
};

/// Remove constness and reference from type T
/// e.g: pure_type_t<const int&> == int
template <typename T> using pure_type_t = typename pure_type<T>::type;

template <typename T> struct is_number_type {
  static constexpr bool value =
      std::is_integral_v<T> || std::is_floating_point_v<T>;
};

template <typename T>
static constexpr bool is_number_type_v = is_number_type<T>::value;

template <typename T, template <typename...> class Template>
struct is_specialization_of : std::integral_constant<bool, false> {};

template <template <typename...> class Template, typename... Args>
struct is_specialization_of<Template<Args...>, Template>
    : std::integral_constant<bool, true> {};

template <typename> struct is_tuple : std::false_type {};

template <typename... T> struct is_tuple<std::tuple<T...>> : std::true_type {};

template <typename T> static constexpr bool is_tuple_v = is_tuple<T>::value;

template <typename> struct is_smart_ptr : std::false_type {};

template <typename T>
struct is_smart_ptr<std::shared_ptr<T>> : std::true_type {};

template <typename T>
struct is_smart_ptr<std::unique_ptr<T>> : std::true_type {};

template <typename T>
static constexpr bool is_smart_ptr_v = is_smart_ptr<T>::value;

template <class...> using to_void = void;

template <typename T, typename = void> struct is_iterable : std::false_type {};
template <typename T>
struct is_iterable<T, to_void<decltype(std::begin(std::declval<T>())),
                              decltype(std::end(std::declval<T>()))>>
    : std::true_type {};
template <typename T>
static constexpr bool is_iterable_v = is_iterable<T>::value;

template <typename T, typename = void>
struct is_back_insertible : std::false_type {};
template <typename T>
struct is_back_insertible<T, to_void<decltype(std::declval<T>().push_back({}))>>
    : std::true_type {};
template <typename T>
static constexpr bool is_back_insertible_v = is_back_insertible<T>::value;

template <typename T, typename = void>
struct is_position_independent_insertible : std::false_type {};
template <typename T>
struct is_position_independent_insertible<
    T, to_void<decltype(std::declval<T>().insert({}))>> : std::true_type {};
template <typename T>
static constexpr bool is_position_independent_insertible_v =
    is_position_independent_insertible<T>::value;

template <typename Container>
using iterator_category_of = typename std::iterator_traits<
    typename Container::iterator>::iterator_category;

template <typename Container>
struct has_random_access_iterator
    : public std::is_base_of<std::random_access_iterator_tag,
                             iterator_category_of<Container>> {};

template <typename Container>
static constexpr bool has_random_access_iterator_v =
    has_random_access_iterator<Container>::value;

template <typename, typename> struct append_to_type_seq {};

template <typename T, typename... Ts>
struct append_to_type_seq<T, std::tuple<Ts...>> {
  using type = std::tuple<Ts..., T>;
};

template <typename... Ts> struct revert_type_seq { using type = std::tuple<>; };

template <typename T, typename... Ts> struct revert_type_seq<T, Ts...> {
  using type =
      typename append_to_type_seq<T,
                                  typename revert_type_seq<Ts...>::type>::type;
};

template <int... Is> struct index_list {};

namespace detail {
template <int MIN, int N, int... Is> struct range_builder;

template <int MIN, int... Is> struct range_builder<MIN, MIN, Is...> {
  typedef index_list<Is...> type;
};

template <int MIN, int N, int... Is>
struct range_builder : public range_builder<MIN, N - 1, N - 1, Is...> {};
} // namespace detail

template <int MIN, int MAX>
using index_range = typename detail::range_builder<MIN, MAX>::type;

template <typename... Args, int... Is>
typename revert_type_seq<Args...>::type revert_tuple(std::tuple<Args...> &&t,
                                                     index_list<Is...>) {
  //    using reverted_tuple = typename revert_type_seq<Args...>::type;

  // Forwarding machinery that handles both lvalues and rvalues...
  //    auto rt = std::forward_as_tuple(
  //            std::forward<
  //                typename std::conditional<
  //                    std::is_lvalue_reference<
  //                        typename std::tuple_element<Is,
  //                        reverted_tuple>::type
  //                        >::value,
  //                    typename std::tuple_element<Is, reverted_tuple>::type,
  //                    typename std::remove_reference<
  //                        typename std::tuple_element<Is,
  //                        reverted_tuple>::type
  //                        >::type
  //                    >::type
  //                >(std::get<sizeof...(Args) - Is - 1>(t))...
  //        );

  //    return rt;
  return std::forward_as_tuple(
      std::get<sizeof...(Args) - Is - 1>(std::move(t))...);
}

template <typename... Args>
typename revert_type_seq<Args...>::type revert_tuple(std::tuple<Args...> &&t) {
  return revert_tuple(std::move(t), index_range<0, sizeof...(Args)>());
}

template <typename... Args>
typename revert_type_seq<Args...>::type make_revert(Args &&... args) {
  return revert_tuple(std::forward_as_tuple(std::forward<Args>(args)...));
}
template <typename F, typename... Args, int... Is>
auto revert_call(F &&f, index_list<Is...>, Args &&... args) {
  auto rt = make_revert(std::forward<Args>(args)...);
  return f(std::get<Is>(rt)...);
}

template <typename F, typename... Args>
auto revert_call(F &&f, Args &&... args) {
  return revert_call(f, index_range<0, sizeof...(Args)>(),
                     std::forward<Args>(args)...);
}

} // namespace nstl
} // namespace maf
#endif // TYPETRAITS_H
