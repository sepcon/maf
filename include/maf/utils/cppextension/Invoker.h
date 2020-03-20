#pragma once

#include <functional>
#include <tuple>

namespace maf {
namespace util {

template <class Callable, class... Args> class Invoker {
  std::tuple<Callable, Args...> _callableAndParams;
  template <class Tuple, size_t... index>
  static void invoke_(Tuple &&t, std::index_sequence<index...>) {
    std::invoke(std::move(std::get<index>(t))...);
  }

public:
  Invoker(Callable &&f, Args &&... args)
      : _callableAndParams{std::forward<Callable>(f),
                           std::forward<Args>(args)...} {}
  void invoke() {
    invoke_(std::move(_callableAndParams),
            std::make_index_sequence<1 + sizeof...(Args)>());
  }
};

template <class Callable, class... Args>
Invoker<std::decay_t<Callable>, std::decay_t<Args>...>
makeInvoker(Callable &&f, Args &&... args) {
  return Invoker<std::decay_t<Callable>, std::decay_t<Args>...>{
      std::forward<Callable>(f), std::forward<Args>(args)...};
}

} // namespace util
} // namespace maf
