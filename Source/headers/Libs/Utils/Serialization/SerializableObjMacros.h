#ifndef SERIALIZABLEOBJMACROS_H
#define SERIALIZABLEOBJMACROS_H

#define _Args(...) __VA_ARGS__
#define STRIP_PARENS(X) X
#define EXPAND(X) STRIP_PARENS( _Args X )


// Make a FOREACH macro
#define FE_1(WHAT, X) WHAT(X)
#define FE_2(WHAT, X, ...) WHAT(X)FE_1(WHAT, __VA_ARGS__)
#define FE_3(WHAT, X, ...) WHAT(X)FE_2(WHAT, __VA_ARGS__)
#define FE_4(WHAT, X, ...) WHAT(X)FE_3(WHAT, __VA_ARGS__)
#define FE_5(WHAT, X, ...) WHAT(X)FE_4(WHAT, __VA_ARGS__)
//... repeat as needed

// Make a FOREACH macro
#define FEI_1(WHAT, X) WHAT(X, 1)
#define FEI_2(WHAT, X, ...) WHAT(X, 2)FEI_1(WHAT, __VA_ARGS__)
#define FEI_3(WHAT, X, ...) WHAT(X, 3)FEI_2(WHAT, __VA_ARGS__)
#define FEI_4(WHAT, X, ...) WHAT(X, 4)FEI_3(WHAT, __VA_ARGS__)
#define FEI_5(WHAT, X, ...) WHAT(X, 5)FEI_4(WHAT, __VA_ARGS__)
//... repeat as needed

#define GET_MACRO(_1,_2,_3,_4,_5,NAME,...) NAME
#define MC_FOR_EACH(action,...) \
  GET_MACRO(__VA_ARGS__,FE_5,FE_4,FE_3,FE_2,FE_1)(action,__VA_ARGS__)

#define MC_FOR_EACH_WITH_INDEX(action, ...) \
  GET_MACRO(__VA_ARGS__,FEI_5,FEI_4,FEI_3,FEI_2,FEI_1)(action,__VA_ARGS__)

#define IGNORE_SECOND_PARAM__(first, second) first,
#define IGNORE_SECOND_PARAM_(...) IGNORE_SECOND_PARAM__(__VA_ARGS__)
#define IGNORE_SECOND_PARAM(firstsecond) IGNORE_SECOND_PARAM_(EXPAND(firstsecond))

#define DECLARE_GET_FUNCTION_(Type, Name, index) \
    public: \
        Type& get##Name() { return std::get<index>(_data); } \
        const Type& get##Name() const { return std::get<index>(_data); }

#define DECLARE_GET_FUNCTION(...) DECLARE_GET_FUNCTION_(__VA_ARGS__)

#define DECLARE_GET_FUNCTION_WITH_INDEX(TypeName, index) \
    DECLARE_GET_FUNCTION( EXPAND(TypeName), index)

#define DEFINE_DATA_AS_REVERT_TUPLE(...) \
        MC_FOR_EACH_WITH_INDEX(DECLARE_GET_FUNCTION_WITH_INDEX, __VA_ARGS__) \
        using value_type = thaf::stl::revert_type_seq<MC_FOR_EACH(IGNORE_SECOND_PARAM, __VA_ARGS__) bool>::type; \
        value_type _data;

#define PROPERTIES_MAP_(...) \
    DEFINE_DATA_AS_REVERT_TUPLE(__VA_ARGS__)


#define SERIALIZABLE_OBJECT_PRV_(ClassName) \
    struct ClassName { \
        friend struct thaf::srz::Serialization<ClassName>; \
        DEFAULT_CONSTRUCTIONS_(ClassName)

#define SERIALIZABLE_OBJECT_END_PRV_(ClassName) };

#define DEFAULT_CONSTRUCTIONS_(ClassName) \
        ClassName() = default; \
        ClassName(const ClassName& rhs) = default; \
        ClassName(ClassName&& rhs) = default; \
        ClassName& operator=(const ClassName& rhs) = default; \
        ClassName& operator=(ClassName&& rhs) = default; \
        template <typename... T> \
        ClassName(T&&... args) : _data{thaf::stl::make_revert(std::forward<T>(args)..., false)} {}


#endif // SERIALIZABLEOBJMACROS_H
