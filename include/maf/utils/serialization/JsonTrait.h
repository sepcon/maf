#pragma once

#include <string>

namespace maf {
namespace srz {


template <class JsonType>
struct JsonTraitBase
{
    using Type = JsonType;
    static constexpr bool isJson() { return true; }
    static Type unmarshall(const std::string& /*s*/) { return {};}
    static std::string marshall(const Type& /*j*/) { return {}; }
    static size_t marshallSize(const Type& /*j*/) { return 0; }

};

template <typename T>
struct JsonTrait{};

template <typename T>
struct is_maf_compatible_json
{
public:
    static constexpr bool value = std::is_base_of_v<JsonTraitBase<T>, JsonTrait<T>>;
};

} // srz
} // thaf

