#pragma once

#include <type_traits>

namespace maf {
namespace srz {

struct Tuplizable{};
template <typename T>
inline auto constexpr is_tuplizable_type_v = std::is_base_of_v<Tuplizable, T>;

}
}
