#pragma once

#include <type_traits>

namespace maf {
namespace messaging {
namespace paco {

template <class PaCo, class T>
inline constexpr bool IsStatus = PaCo::template IsStatus<T>;
template <class PaCo, class T>
inline constexpr bool IsOutput = PaCo::template IsOutput<T>;
template <class PaCo, class T>
inline constexpr bool IsInput = PaCo::template IsInput<T>;
template <class PaCo, class T>
inline constexpr bool IsAttributes = PaCo::template IsAttributes<T>;
template <class PaCo, class T>
inline constexpr bool IsSignal = PaCo::template IsSignal<T>;
template <class PaCo, class T>
inline constexpr bool IsRequest = PaCo::template IsRequest<T>;
template <class PaCo, class T>
inline constexpr bool IsProperty = PaCo::template IsProperty<T>;

template <class PaCo, class T>
using AllowOnlyStatusT = typename std::enable_if_t<IsStatus<PaCo, T>, bool>;

template <class PaCo, class T>
using AllowOnlyInputT = std::enable_if_t<IsInput<PaCo, T>, bool>;

template <class PaCo, class T>
using AllowOnlyOutputT = std::enable_if_t<IsOutput<PaCo, T>, bool>;

template <class PaCo, class T>
using AllowOnlyAttributesT = std::enable_if_t<IsAttributes<PaCo, T>, bool>;

template <class PaCo, class T>
using AllowOnlySignalT = std::enable_if_t<IsSignal<PaCo, T>, bool>;

template <class PaCo, class T>
using AllowOnlyRequestT = std::enable_if_t<IsRequest<PaCo, T>, bool>;

template <class PaCo, class T>
using AllowOnlyPropertyT = std::enable_if_t<IsProperty<PaCo, T>, bool>;

template <class PaCo, class T>
using AllowOnlyRequestOrOutputT =
    std::enable_if_t<IsRequest<PaCo, T> || IsOutput<PaCo, T>, bool>;

template <class PaCo, class T>
using AllowOnlyRequestOrInputT =
    std::enable_if_t<IsRequest<PaCo, T> || IsInput<PaCo, T>, bool>;

} // namespace paco
} // namespace messaging
} // namespace maf
