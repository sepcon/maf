#pragma once

#include <string>

namespace maf {
namespace srz {

template <class JsonType>
struct JsonTraitBase
{

    static JsonType unmarshall(const std::string& /*s*/) { return {};}
    static std::string marshall(const JsonType& /*j*/) { return {}; }
    static size_t marshallSize(const JsonType& /*j*/) { return 0; }

protected:
    JsonTraitBase();
};

template <class JsonType>
struct JsonTrait : public JsonTraitBase<JsonType>
{
protected:
    JsonTrait();
};

class MyJson{};
template <>
struct JsonTrait<MyJson> : public JsonTraitBase<MyJson>{};

} // srz
} // thaf

