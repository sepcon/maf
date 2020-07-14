#pragma once
/**
 * @brief Json11Trait.h provides implementation for JsonTrait<json11::Json>
 * that allows maf::srz::SerializationTrait<Json11::Json> and
 * maf::srz::DumpHelper<json11::Json> to work and allow using macros to generate
 * classes that can read data from json/json_string with json11 is helper to
 * parse and convey the json data To enable this support, you have to define
 * macro `MAF_USING_JSON11` and include "json11.hpp" header before include this
 * file. it'd better having some file like this: HEADER: `MyJson11.h` #define
 * MAF_USING_JSON11 #include "json11.hpp" #include "Json11Trait.h"
 */

#define MAF_ENABLE_JSON

#include <maf/utils/cppextension/TupleManip.h>
#include <maf/utils/cppextension/TypeTraits.h>
#include <maf/utils/serialization/TuplizableTypeBase.h>
#include <maf/utils/serialization/JsonTrait.h>

//#include "json11.hpp"

namespace maf {
namespace srz {

using Json = json11::Json;

template <typename T>
struct is_json11_convertible_type
    : public std::integral_constant<bool, std::is_same_v<std::string, T> ||
                                              nstl::is_number_type_v<T> ||
                                              is_tuplizable_type_v<T>> {};

template <typename T>
struct is_json11_convertible_type<std::vector<T>>
    : public std::integral_constant<
          bool, is_json11_convertible_type<
                    typename std::vector<T>::value_type>::value> {};

template <typename Key, typename Value>
struct is_json11_convertible_type<std::map<Key, Value>>
    : public std::integral_constant<
          bool, std::is_same_v<std::string, Key> &&
                    is_json11_convertible_type<Value>::value> {};

template <typename Type> struct J11TraitImpl {
  static Type get(const Json &j) { return internal::template get<Type>(j); }

  static bool exist(const Json &j) { return internal::template exist<Type>(j); }

  struct internal {
    template <typename T, std::enable_if_t<is_tuplizable_type_v<T>, bool> = true>
    static T get(const Json &j) {
      T t;
      t.load_from_json(j);
      return t;
    }

    template <typename T, std::enable_if_t<is_tuplizable_type_v<T>, bool> = true>
    static bool exist(const Json &j) {
      return j.is_object();
    }
  };
};

template <> struct J11TraitImpl<json11::Json> {
  static json11::Json get(const Json &j) { return j; }
  static bool exist(const Json &) { return true; }
};

template <class T> struct J11TraitImpl<std::vector<T>> {
  using ReturnType = std::vector<T>;
  static_assert(is_json11_convertible_type<T>::value,
                R"(Please provide: vector of below types only:
    1. std::string
    2. integer types
    3. double
    4. bool
    5. TuplizableTypeBase classes
    6. std::vector<one_of_above_types>
    7. std::map<string, one_of_above_types>)");

  static ReturnType get(const Json &j) {
    auto &array = j.array_items();
    ReturnType ret;
    for (auto &item : array) {
      ret.emplace_back(J11TraitImpl<T>::get(item));
    }
    return ret;
  }
  static bool exist(const Json &j) { return j.is_array(); }
};

template <class Key, class Value> struct J11TraitImpl<std::map<Key, Value>> {
  using ReturnType = std::map<Key, Value>;
  static_assert(
      std::is_same_v<std::string, Key> &&
          is_json11_convertible_type<Value>::value,
      R"(Please provide: map of KEY `string` and VALUE in below types only:
    1. std::string
    2. integer types
    3. double
    4. bool
    5. TuplizableTypeBase classes
    6. std::vector<one_of_above_types>
    7. std::map<string, one_of_above_types>)");

  static ReturnType get(const Json &j) {
    auto &items = j.object_items();
    ReturnType ret;
    for (auto &item : items) {
      ret.insert(
          std::make_pair(item.first, J11TraitImpl<Value>::get(item.second)));
    }
    return ret;
  }
  static bool exist(const Json &j) { return j.is_object(); }
};

#define mc_J11TraitImpl(type, exist_function, get_function)                    \
  template <> struct J11TraitImpl<type> {                                      \
    static type get(const Json &j) {                                           \
      return static_cast<type>(j.get_function());                              \
    }                                                                          \
    static bool exist(const Json &j) { return j.exist_function(); }            \
  };

mc_J11TraitImpl(int16_t, is_number, int_value)
    mc_J11TraitImpl(uint16_t, is_number, int_value)
        mc_J11TraitImpl(int32_t, is_number, int_value)
            mc_J11TraitImpl(uint32_t, is_number, int_value)
                mc_J11TraitImpl(int64_t, is_number, number_value)
                    mc_J11TraitImpl(uint64_t, is_number, number_value)
                        mc_J11TraitImpl(float, is_number, number_value)
                            mc_J11TraitImpl(double, is_number, number_value)
                                mc_J11TraitImpl(std::string, is_string,
                                                string_value)
                                    mc_J11TraitImpl(bool, is_bool, bool_value)

#undef mc_J11TraitImpl

                                        template <>
                                        struct JsonTrait<json11::Json>
    : public JsonTraitBase<json11::Json> {
  using Json = json11::Json;
  static json11::Json unmarshall(const std::string &s) {
    std::string err;
    return Json::parse(s, err);
  }
  static std::string marshall(const Json &j) { return j.dump(); }
  static size_t marshallSize(const Json &j) { return j.dump().size(); }

  template <typename T> static bool has(const Json &j, const std::string &key) {
    return J11TraitImpl<T>::exist(j[key]);
  }

  template <typename T> static T get(const Json &j, const std::string &key) {
    return static_cast<T>(J11TraitImpl<T>::get(j[key]));
  }

  static Json fromString(const std::string &json_string) {
    std::string err;
    return json11::Json::parse(json_string, err);
  }
};

} // namespace srz
} // namespace maf
