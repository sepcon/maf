#ifndef SERIALIZABLEOBJMACROS_H
#define SERIALIZABLEOBJMACROS_H

#include <maf/utils/cppextension/Loop.mc.h>
#include <maf/utils/serialization/BasicTypes.h>

#ifdef MAF_ENABLE_JSON
#   include <maf/utils/serialization/JsonTrait.h>
#endif

/// ---------------------------------------------------------------------------------------------------------------------------------------
/// ----------------------------Declare Serializable class --------------------------------------------------------------------------------
/// ---------------------------------------------------------------------------------------------------------------------------------------
#define mc_tuple_like_object_(ClassName) \
struct ClassName : public maf::srz::TupleLikeBase<ClassName> { \
    friend struct maf::srz::SerializationTrait<ClassName>; \
    mc_constructors_(ClassName)

#define mc_tuple_like_object_end_(ClassName) \
public: \
    static std::shared_ptr<ClassName> create(data_type data) { \
        auto p = std::make_shared<ClassName>(); \
        p->_data = std::move(data); \
        return p; \
    } \
    static std::shared_ptr<ClassName> create() { \
        return std::make_shared<ClassName>(); \
    } \
};

#define mc_tuple_like_object_has_base_(ClassName, BaseClassName) \
struct ClassName : \
public maf::srz::TupleLikeBase<ClassName>, \
    public BaseClassName { \
    static_assert(std::is_default_constructible_v<BaseClassName>, "The class `"  #BaseClassName "` which is specified as base of `" #ClassName "` must be has default constructor" ); \
    friend struct maf::srz::SerializationTrait<ClassName>; \
    mc_constructors_(ClassName)

#define mc_tuple_like_object_has_base_end_(ClassName) mc_tuple_like_object_end_(ClassName)

#define mc_declare_json_get_function
/// ---------------------------------------------------------------------------------------------------------------------------------------
/// ----------------------------Declare std::tuple<...> for storing properties of class----------------------------------------------------
/// ---------------------------------------------------------------------------------------------------------------------------------------
#define mc_properties_map_(...) \
public: \
    using data_type = std::tuple< mc_remove_first_arg( mc_for_each(mc_take_only_first_arg, __VA_ARGS__) )>; \
    mc_for_each_with_index( mc_declare_get_set_funcs_with_index, __VA_ARGS__) \
    msvc_expand_va_args( define_dump_function(__VA_ARGS__) ) \
    msvc_expand_va_args( define_load_from_json_functions(__VA_ARGS__) ) \
    data_type& tuple() { return _data; } \
    const data_type& tuple () const { return _data; } \
private: \
    static constexpr size_t __propertiesCount = std::tuple_size_v<data_type>; \
    data_type _data = data_type{ mc_remove_first_arg( mc_for_each( mc_get_default_value, __VA_ARGS__ ) ) }; \
    mc_define_dump_helper_func


#define mc_take_2_first_params_param_(first, second, ...) /*(*/ first, second /*)*/
#define mc_take_2_first_params_param(...) msvc_expand_va_args ( mc_take_2_first_params_param_( __VA_ARGS__ ) )

#define mc_take_only_first_arg__(first, ...) /*(*/ , first /*)*/
#define mc_take_only_first_arg_(...) msvc_expand_va_args( mc_take_only_first_arg__(__VA_ARGS__) )
#define mc_take_only_first_arg(firstsecond) mc_take_only_first_arg_( mc_strip_parentheses(firstsecond) )

#define mc_no_default(type, name)                   ,{}
#define mc_has_default(type, name, TheDefaultValue)   ,TheDefaultValue
#define mc_get_choose_correct_default_value_macro(_1, _2, candidate1, candidate, ...) candidate
#define mc_get_default_value_(...) msvc_expand_va_args( mc_get_choose_correct_default_value_macro(__VA_ARGS__, mc_has_default, mc_no_default)(__VA_ARGS__) )
#define mc_get_default_value(SetOfParams) mc_get_default_value_( mc_strip_parentheses(SetOfParams) )


/// ---------------------------------------------------------------------------------------------------------------------------------------
/// ----------------------------Macros to generate dump functions for class ClassName------------------------------------------------------
/// ---------------------------------------------------------------------------------------------------------------------------------------

#ifdef MAF_ENABLE_DUMP

#define mc_define_dump_helper_func \
    template<typename T> \
    static void dump(const char* valName, const T& val, int level, std::string& strOut) { \
        strOut += maf::srz::getIndent(level, true); \
        maf::srz::dump(valName, level, strOut); \
        strOut += " : "; \
        maf::srz::dump(val, level, strOut); \
        strOut += ","; \
    }

#   define mc_dump_each_property_(Type, Name, index) \
        dump(#Name, Name(), level + 1, strOut);

#   define mc_dump_each_property(...) msvc_expand_va_args( mc_dump_each_property_(__VA_ARGS__) )

#   define mc_dump_each_property_with_index(TypeName, index) \
       mc_dump_each_property( mc_take_2_first_params_param( mc_strip_parentheses(TypeName) ), index)

#   define define_dump_function(...) \
       void dump(int level, std::string& strOut) const { \
           strOut += "{"; \
           mc_for_each_with_index( mc_dump_each_property_with_index, __VA_ARGS__) \
           if(strOut.back() == ',') { \
               strOut.resize(strOut.size() - 1); \
           } \
           strOut += maf::srz::getIndent(level, true) + "}"; \
       } \
       std::string dump(int level = 0) const {\
           std::string output; \
           dump(level, output); \
           return output; \
       }
#else
#define mc_define_dump_helper_func
#   define define_dump_function(...) \
        void dump(int /*level*/, std::string& strOut) const { strOut = "Object Dump is not supported"; } \
        std::string dump(int /*level*/ = 0) const { return "Object Dump is not supported"; }
#endif

/// ---------------------------------------------------------------------------------------------------------------------------------------
/// ----------------------------Generate get/set fucntions for each property---------------------------------------------------------------
/// ---------------------------------------------------------------------------------------------------------------------------------------
#define mc_declare_get_set_funcs_with_index(TypeName, index) \
    mc_declare_get_set_function( mc_take_2_first_params_param ( mc_strip_parentheses(TypeName) ), index)

#define mc_declare_get_set_function(...) msvc_expand_va_args( mc_declare_get_set_funcs_(__VA_ARGS__) )

#define mc_value_at_r(index) std::get< __propertiesCount - index>(_data)
#define mc_declare_get_set_funcs_(Type, Name, index) \
public: \
    void set_##Name(const Type& value) { Name() = value; } \
    void set_##Name(Type&& value) { Name() = std::move(value); } \
    const Type& Name() const { return mc_value_at_r(index); } \
    Type& Name() { return mc_value_at_r(index); }


/// ---------------------------------------------------------------------------------------------------------------------------------------
/// ----------------------------Macros to load data from json ---------------------------------------------------------------------------
/// ---------------------------------------------------------------------------------------------------------------------------------------

#ifdef MAF_ENABLE_JSON
#   define mc_load_json_on_each_property_(Type, Name, index) \
      auto& Name##__ = Name(); \
      if(TheJsonTrait::template has<Type>(jsonIn, #Name)) { \
           Name##__ = TheJsonTrait::template get<Type>(jsonIn, #Name); \
      }

#   define mc_load_from_json_each_property(...) msvc_expand_va_args( mc_load_json_on_each_property_(__VA_ARGS__) )

#   define mc_load_json_on_each_property_with_index(TypeName, index) \
       mc_load_from_json_each_property( mc_take_2_first_params_param( mc_strip_parentheses(TypeName) ), index)

#   define define_load_from_json_functions(...) \
       template<typename JsonType, std::enable_if_t<maf::srz::is_maf_compatible_json<JsonType>::value, bool> = true> \
       void load_from_json(const JsonType& jsonIn) { \
           using TheJsonTrait = maf::srz::JsonTrait<JsonType>; \
           mc_for_each_with_index( mc_load_json_on_each_property_with_index, __VA_ARGS__) \
       } \
       template<typename JsonType, std::enable_if_t<maf::srz::is_maf_compatible_json<JsonType>::value, bool> = true> \
       void load_from_json(const std::string& json_string) {\
           using TheJsonTrait = maf::srz::JsonTrait<JsonType>; \
           load_from_json(TheJsonTrait::fromString(json_string)); \
       }
#else
#   define define_load_from_json_functions(...) \
       template<typename JsonType, std::enable_if_t<maf::srz::is_maf_compatible_json<JsonType>::value, bool> = true> \
       void load_from_json(const JsonType& /*jsonIn*/) { } \
       template<typename JsonType, std::enable_if_t<maf::srz::is_maf_compatible_json<JsonType>::value, bool> = true> \
       void load_from_json(const std::string& /*json_string*/) { }
#endif



#define mc_constructors_(ClassName) \
    ClassName() = default; \
    ClassName(const ClassName& rhs) = default; \
    ClassName(ClassName&& rhs) = default; \
    ClassName& operator=(const ClassName& rhs) = default; \
    ClassName& operator=(ClassName&& rhs) = default; \
    template <typename... T> \
    ClassName(T... args) : _data(std::move(args)...) {} \
    friend bool operator==(const ClassName& lhs, const ClassName& rhs) { return rhs._data == lhs._data; } \
    friend bool operator<=(const ClassName& lhs, const ClassName& rhs) { return rhs._data <= lhs._data; } \
    friend bool operator>=(const ClassName& lhs, const ClassName& rhs) { return rhs._data >= lhs._data; } \
    friend bool operator<(const ClassName& lhs, const ClassName& rhs) { return rhs._data < lhs._data;   } \
    friend bool operator>(const ClassName& lhs, const ClassName& rhs) { return rhs._data > lhs._data;   } \
    friend bool operator!=(const ClassName& lhs, const ClassName& rhs) { return rhs._data != lhs._data; }


#endif // SERIALIZABLEOBJMACROS_H
