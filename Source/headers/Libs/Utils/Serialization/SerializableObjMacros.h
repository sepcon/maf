#ifndef SERIALIZABLEOBJMACROS_H
#define SERIALIZABLEOBJMACROS_H

#include "headers/Libs/Utils/CppExtension/Macros.h"

#define IGNORE_SECOND_PARAM__(first, second) first ,
#define IGNORE_SECOND_PARAM_(...) MSVC_EXPAND_VA_ARGS( IGNORE_SECOND_PARAM__(__VA_ARGS__) )
#define IGNORE_SECOND_PARAM(firstsecond) IGNORE_SECOND_PARAM_( STRIP_PARENTHESES(firstsecond) )

#define DUMP_EACH_PROPERTY_(Type, Name, index) \
        strOut += thaf::srz::getIndent(level + 1, true); \
        thaf::srz::DumpHelper<std::pair<std::string, Type>>::dump(std::make_pair(std::string( #Name ), get##Name() ), level + 1, strOut); \
        strOut += ",";

#define DUMP_EACH_PROPERTY(...) MSVC_EXPAND_VA_ARGS( DUMP_EACH_PROPERTY_(__VA_ARGS__) )

#define DUMP_EACH_PROPERTY_WITH_INDEX(TypeName, index) \
    DUMP_EACH_PROPERTY( STRIP_PARENTHESES(TypeName), index)

#define DEFINE_DUMP_FUNCTION(...) \
        void dump(int level, std::string& strOut) const { \
            strOut += "{"; \
            MC_FOR_EACH_WITH_INDEX( DUMP_EACH_PROPERTY_WITH_INDEX, __VA_ARGS__) \
            strOut += thaf::srz::getIndent(level, true) + "}"; \
        } \
        std::string dump(int level) const {\
            std::string output; \
            dump(level, output); \
            return output; \
        }

#define DECLARE_GET_FUNCTION_(Type, Name, index) \
    public: \
        void set##Name(const Type& value) { std::get<index>(_data) = value; } \
		void set##Name(Type&& value) { std::get<index>(_data) = std::move(value); } \
        const Type& get##Name() const { return std::get<index>(_data); }

#define DECLARE_GET_FUNCTION(...) MSVC_EXPAND_VA_ARGS( DECLARE_GET_FUNCTION_(__VA_ARGS__) )

#define DECLARE_GET_FUNCTION_WITH_INDEX(TypeName, index) \
    DECLARE_GET_FUNCTION( STRIP_PARENTHESES(TypeName), index)


#define PROPERTIES_MAP_(...) \
        MC_FOR_EACH_WITH_INDEX( DECLARE_GET_FUNCTION_WITH_INDEX, __VA_ARGS__) \
        using value_type = thaf::stl::revert_type_seq< MC_FOR_EACH(IGNORE_SECOND_PARAM, __VA_ARGS__) bool>::type; \
        value_type _data; \
        MSVC_EXPAND_VA_ARGS( DEFINE_DUMP_FUNCTION(__VA_ARGS__) )



#define SERIALIZABLE_OBJECT_PRV_(ClassName) \
    struct ClassName { \
        friend struct thaf::srz::Serialization<ClassName>; \
        DEFAULT_CONSTRUCTIONS_(ClassName) \
        constexpr const char* name() const { return #ClassName; }

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
