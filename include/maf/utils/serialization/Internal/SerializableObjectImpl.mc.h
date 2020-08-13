#pragma once

#include <maf/utils/cppextension/Loop.mc.h>
#include <maf/utils/serialization/Tuplizable.h>

#define mc_maf_sb_object(name) struct name : public maf::srz::Tuplizable {
#define mc_maf_sb_object_ex(name, base) \
  struct name : public maf::srz::Tuplizable, public base {
#define mc_maf_sb_endobject(name)                                        \
 public:                                                                 \
  mc_maf_sb_define_constructors(name) bool operator==(const name &other) \
      const {                                                            \
    return as_tuple() == other.as_tuple();                               \
  }                                                                      \
  bool operator!=(const name &other) const { return !(*this == other); } \
  bool operator<(const name &other) const {                              \
    return this->as_tuple() < other.as_tuple();                          \
  }                                                                      \
  }                                                                      \
  ;

#define mc_maf_sb_members(...)                                           \
  mc_maf_sb_define_as_tuple_funcs(__VA_ARGS__)                           \
      mc_maf_sb_define_get_set_funcs(__VA_ARGS__)                        \
          mc_maf_sb_define_dump_functions(__VA_ARGS__)                   \
              mc_maf_sb_define_set_all_function(__VA_ARGS__)             \
                  mc_maf_sb_define_load_from_json_functions(__VA_ARGS__) \
                      mc_maf_sb_declare_member_vars(__VA_ARGS__)

#define mc_maf_sb_define_constructors(name) \
 public:                                    \
  template <class... Args>                  \
  name(Args... args) {                      \
    set_all(std::forward<Args>(args)...);   \
  }                                         \
  name() = default;                         \
  name(const name &other) = default;        \
  name(name &&other) = default;             \
  name &operator=(name &&other) = default;  \
  name &operator=(const name &other) = default;

#define mc_maf_sb_define_set_all_function(...)                          \
 public:                                                                \
  void set_all(mc_maf_sb_remove_first_arg(                              \
      mc_maf_for_each(mc_maf_sb_declare_function_param, __VA_ARGS__))){ \
      mc_maf_for_each(mc_maf_sb_set_member_value, __VA_ARGS__)};

#define mc_maf_sb_declare_function_param(parentheses) \
  mc_maf_sb_declare_function_param_no_prt(mc_strip_parentheses(parentheses))

#define mc_maf_sb_declare_function_param_no_prt(...)          \
  mc_maf_msvc_expand_va_args(mc_maf_sb_get_overload_3_params( \
      __VA_ARGS__, mc_maf_sb_declare_function_param_default,  \
      mc_maf_sb_declare_function_param_no_default)(__VA_ARGS__))
#define mc_maf_sb_declare_function_param_no_default(type, name) , type name = {}
#define mc_maf_sb_declare_function_param_default(type, name, default_val) \
  , type name = default_val

#define mc_maf_sb_set_member_value(parentheses) \
  mc_maf_sb_set_member_value_impl(mc_strip_parentheses(parentheses))
#define mc_maf_sb_set_member_value_impl(...) \
  mc_maf_msvc_expand_va_args(mc_maf_sb_set_member_value_impl_(__VA_ARGS__))
#define mc_maf_sb_set_member_value_impl_(type, name, ...) \
  mc_maf_sb_get_member_var_name(name) = std::move(name);

#define mc_maf_sb_take_2_first_params_param_(first, second, ...) /*(*/ \
  first, second                                                  /*)*/
#define mc_maf_sb_take_2_first_params_param(...) \
  mc_maf_msvc_expand_va_args(mc_maf_sb_take_2_first_params_param_(__VA_ARGS__))

#define mc_maf_sb_take_only_first_arg__(first, ...) /*(*/ , first /*)*/
#define mc_maf_sb_take_only_first_arg_(...) \
  mc_maf_msvc_expand_va_args(mc_maf_sb_take_only_first_arg__(__VA_ARGS__))
#define mc_maf_sb_take_only_first_arg(firstsecond) \
  mc_maf_sb_take_only_first_arg_(mc_strip_parentheses(firstsecond))

#define mc_maf_sb_define_get_set_funcs(...) \
  mc_maf_for_each(mc_maf_sb_define_get_set_func, __VA_ARGS__)

#define mc_maf_sb_define_get_set_func(parentheses) \
  mc_maf_sb_define_get_set_func_impl(mc_strip_parentheses(parentheses))
#define mc_maf_sb_define_get_set_func_impl(...) \
  mc_maf_msvc_expand_va_args(mc_maf_sb_define_get_set_func_impl_(__VA_ARGS__))

#define mc_maf_sb_define_get_set_func_impl_(type, name, ...) \
 public:                                                     \
  void set_##name(type &&name) {                             \
    mc_maf_sb_get_member_var_name(name) = std::move(name);   \
  }                                                          \
  void set_##name(const type &name) {                        \
    mc_maf_sb_get_member_var_name(name) = name;              \
  }                                                          \
  const type &get_##name() const {                           \
    return mc_maf_sb_get_member_var_name(name);              \
  }                                                          \
  type &get_##name() { return mc_maf_sb_get_member_var_name(name); }

#define mc_maf_sb_declare_member_vars(...) \
 private:                                  \
  mc_maf_for_each(mc_maf_sb_declare_member_vars_impl, __VA_ARGS__)

#define mc_maf_sb_declare_member_vars_impl(parentheses) \
  mc_maf_sb_declare_member_vars_no_prts(mc_strip_parentheses(parentheses))

#define mc_maf_sb_declare_member_vars_no_prts(...)            \
  mc_maf_msvc_expand_va_args(mc_maf_sb_get_overload_3_params( \
      __VA_ARGS__, mc_maf_sb_declare_member_var_default,      \
      mc_maf_sb_declare_member_var_no_default)(__VA_ARGS__))

#define mc_maf_sb_declare_member_var_no_default(type, name) \
  type mc_maf_sb_get_member_var_name(name);

#define mc_maf_sb_declare_member_var_default(type, name, default_val) \
  type mc_maf_sb_get_member_var_name(name) = default_val;

#define mc_maf_sb_define_as_tuple_funcs(...)                      \
 public:                                                          \
  decltype(auto) as_tuple() {                                     \
    return std::tie(mc_maf_sb_remove_first_arg(mc_maf_for_each(   \
        mc_maf_sb_get_member_var_name_with_comma, __VA_ARGS__))); \
  }                                                               \
  decltype(auto) as_tuple() const {                               \
    return std::tie(mc_maf_sb_remove_first_arg(mc_maf_for_each(   \
        mc_maf_sb_get_member_var_name_with_comma, __VA_ARGS__))); \
  }

#define mc_maf_sb_get_member_var_name_with_comma(parentheses) \
  , mc_maf_sb_get_member_var_name(mc_maf_sb_take_second_param(parentheses))

#define mc_maf_sb_get_member_var_name(name) mc_maf_sb_get_member_var_name_(name)
#define mc_maf_sb_get_member_var_name_(name) name##_

#define mc_maf_sb_take_second_param(parentheses) \
  mc_maf_sb_take_second_param_impl(mc_strip_parentheses(parentheses))
#define mc_maf_sb_take_second_param_impl(...) \
  mc_maf_msvc_expand_va_args(mc_maf_sb_take_second_param_impl_(__VA_ARGS__))
#define mc_maf_sb_take_second_param_impl_(first, second, ...) second

#ifndef MAF_DISABLE_DUMP
#define mc_maf_sb_define_dump_functions(...)                      \
  std::string dump(int indent = -1) {                             \
    std::string out;                                              \
    dump(indent, out);                                            \
    return out;                                                   \
  }                                                               \
  void dump(int indent, std::string &strOut) const {              \
    strOut = maf::srz::getIndent(indent) + "{";                   \
    mc_maf_for_each(mc_maf_sb_dump_each_member,                   \
                    __VA_ARGS__) if (strOut.back() == ',') {      \
      strOut.resize(strOut.size() - 1);                           \
    }                                                             \
    strOut += maf::srz::getIndent(indent, true) + "}";            \
  }                                                               \
                                                                  \
 private:                                                         \
  template <typename T>                                           \
  static void dump(const char *valName, const T &val, int indent, \
                   std::string &strOut) {                         \
    strOut += maf::srz::getIndent(indent, true);                  \
    maf::srz::dump(valName, indent, strOut);                      \
    strOut += maf::srz::keyValueSeparator(indent);                \
    maf::srz::dump(val, indent, strOut);                          \
    strOut += ",";                                                \
  }

#define mc_maf_sb_dump_each_member(parentheses) \
  mc_maf_sb_dump_each_member_impl(mc_maf_sb_take_second_param(parentheses))
#define mc_maf_sb_dump_each_member_impl(name) \
  mc_maf_sb_dump_each_member_impl_(name)
#define mc_maf_sb_dump_each_member_impl_(name) \
  dump(#name, get_##name(), maf::srz::nextLevel(indent), strOut);

#else
#define mc_maf_sb_define_dump_functions(...)    \
  void dump(int, const std::string &) {}        \
  std::string dump(int /*indent*/ = -1) const { \
    return "Object Dump is not supported";      \
  }
#endif

#define mc_maf_sb_remove_first_arg(...) \
  mc_maf_msvc_expand_va_args(mc_maf_sb_remove_first_arg_(__VA_ARGS__))
#define mc_maf_sb_remove_first_arg_(first, ...) __VA_ARGS__
#define mc_maf_sb_get_overload_3_params(_1, _2, candidate1, candidate, ...) \
  candidate

#ifndef MAF_DISABLE_JSON
#include <maf/utils/serialization/JsonTrait.h>
#define mc_maf_sb_define_load_from_json_functions(...)                    \
  template <                                                              \
      typename JsonType,                                                  \
      std::enable_if_t<maf::srz::is_maf_compatible_json<JsonType>::value, \
                       bool> = true>                                      \
  void load_from_json(const JsonType &jsonIn) {                           \
    using TheJsonTrait = maf::srz::JsonTrait<JsonType>;                   \
    mc_maf_for_each(mc_maf_sb_load_json_on_each_member, __VA_ARGS__)      \
  }                                                                       \
  template <                                                              \
      typename JsonType,                                                  \
      std::enable_if_t<maf::srz::is_maf_compatible_json<JsonType>::value, \
                       bool> = true>                                      \
  void load_from_json(const std::string &json_string) {                   \
    using TheJsonTrait = maf::srz::JsonTrait<JsonType>;                   \
    load_from_json(TheJsonTrait::fromString(json_string));                \
  }

#define mc_maf_sb_load_json_on_each_member(TypeName) \
  mc_maf_sb_load_from_json_each_property(            \
      mc_maf_sb_take_2_first_params_param(mc_strip_parentheses(TypeName)))

#define mc_maf_sb_load_from_json_each_property(...) \
  mc_maf_msvc_expand_va_args(mc_maf_sb_load_json_on_each_member_(__VA_ARGS__))

#define mc_maf_sb_load_json_on_each_member_(type, name)  \
  if (TheJsonTrait::template has<type>(jsonIn, #name)) { \
    mc_maf_sb_get_member_var_name(name) =                \
        TheJsonTrait::template get<type>(jsonIn, #name); \
  }

#else

#define mc_maf_sb_define_load_from_json_functions(...)                    \
  template <                                                              \
      typename JsonType,                                                  \
      std::enable_if_t<maf::srz::is_maf_compatible_json<JsonType>::value, \
                       bool> = true>                                      \
  void load_from_json(const JsonType & /*jsonIn*/) {}                     \
  template <                                                              \
      typename JsonType,                                                  \
      std::enable_if_t<maf::srz::is_maf_compatible_json<JsonType>::value, \
                       bool> = true>                                      \
  void load_from_json(const std::string & /*json_string*/) {}
#endif
