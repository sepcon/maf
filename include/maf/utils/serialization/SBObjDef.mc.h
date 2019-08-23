#ifndef SERIALIZABLEOBJMACROS_H
#define SERIALIZABLEOBJMACROS_H

#include "maf/utils/cppextension/Loop.mc.h"

/// ---------------------------------------------------------------------------------------------------------------------------------------
/// ----------------------------Declare Serializable class --------------------------------------------------------------------------------
/// ---------------------------------------------------------------------------------------------------------------------------------------
#define mc_serializable_object_(ClassName) \
    struct ClassName { \
    friend struct maf::srz::SerializationTrait<ClassName>; \
    constexpr const char* class_name() const { return #ClassName; } \
    mc_constructors_(ClassName)

#define mc_serializable_object_end_(ClassName) \
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

#define mc_serializable_object_has_base_(ClassName, BaseClassName) \
    struct ClassName : public BaseClassName { \
    static_assert(std::is_default_constructible_v<BaseClassName>, "The class `"  #BaseClassName "` which is specified as base of `" #ClassName "` must be has default constructor" ); \
    friend struct maf::srz::SerializationTrait<ClassName>; \
    constexpr const char* class_name() const { return #ClassName; } \
    mc_constructors_(ClassName)

#define mc_serializable_object_has_base_end_(ClassName) mc_serializable_object_end_(ClassName)

/// ---------------------------------------------------------------------------------------------------------------------------------------
/// ----------------------------Declare std::tuple<...> for storing properties of class----------------------------------------------------
/// ---------------------------------------------------------------------------------------------------------------------------------------
#define mc_properties_map_(...) \
    public: \
    using data_type = std::tuple< mc_remove_first_arg( mc_for_each(mc_take_only_first_arg, __VA_ARGS__) )>; \
    mc_for_each_with_index( mc_declare_get_set_funcs_with_index, __VA_ARGS__) \
    msvc_expand_va_args( define_dump_function(__VA_ARGS__) ) \
    private: \
    data_type _data = data_type( mc_remove_first_arg( mc_for_each( mc_get_default_value, __VA_ARGS__ ) ) );


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
#define mc_dump_each_property_(Type, Name, index) \
    strOut += maf::srz::getIndent(level + 1, true); \
    maf::srz::DumpHelper<const char*>::dump(#Name, level + 1, strOut); \
    strOut += " : "; \
    auto& Name##__ = get_##Name(); \
    maf::srz::DumpHelper<PURE_TYPE(Name##__)>::dump(Name##__, level + 1, strOut); \
    strOut += ",";

#define mc_dump_each_property(...) msvc_expand_va_args( mc_dump_each_property_(__VA_ARGS__) )

#define mc_dump_each_property_with_index(TypeName, index) \
    mc_dump_each_property( mc_take_2_first_params_param( mc_strip_parentheses(TypeName) ), index)


#ifdef MAF_ENABLE_DUMP
#define define_dump_function(...) \
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
#define define_dump_function(...) \
    void dump(int /*level*/, std::string& /*strOut*/) const { } \
    std::string dump(int /*level*/ = 0) const { return ""; }
#endif

/// ---------------------------------------------------------------------------------------------------------------------------------------
/// ----------------------------Generate get/set fucntions for each property---------------------------------------------------------------
/// ---------------------------------------------------------------------------------------------------------------------------------------
#define mc_declare_get_set_funcs_with_index(TypeName, index) \
    mc_declare_get_function( mc_take_2_first_params_param ( mc_strip_parentheses(TypeName) ), index)

#define mc_declare_get_function(...) msvc_expand_va_args( mc_declare_get_set_funcs_(__VA_ARGS__) )

#define mc_value_at_r(index) std::get< std::tuple_size_v<data_type> - index>(_data)
#define mc_declare_get_set_funcs_(Type, Name, index) \
    public: \
    void set_##Name(const Type& value) { mc_value_at_r(index) = value; } \
    void set_##Name(Type&& value) { mc_value_at_r(index) = std::move(value); } \
    const Type& get_##Name() const { return mc_value_at_r(index); }


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
