#pragma once

#include "maf/utils/cppextension/TupleManip.h"
#include "maf/utils/cppextension/TypeTraits.h"
#include "BasicTypes.h"
#include "JsonTrait.h"
#include <string>
#include <cstring>
#include <cassert>
#include <locale>

#define mc_enable_if_is_tuplelike_(TypeName) template<typename TypeName, std::enable_if_t<is_tuple_like_v<TypeName>, bool> = true>
#define mc_enable_if_is_number_(NumberType) template<typename NumberType, std::enable_if_t<nstl::is_number_type_v<NumberType> , bool> = true>
#define mc_enable_if_is_smartptr_(SmartPtrType) template<typename SmartPtrType, std::enable_if_t<nstl::is_smart_ptr_v<SmartPtrType>, bool> = true>
#define mc_enable_if_is_ptr_(PointerType) template<typename PointerType, std::enable_if_t<std::is_pointer_v<PointerType>, bool> = true>
#define mc_enable_if_is_enum_(EnumType) template<typename EnumType, std::enable_if_t<std::is_enum_v<EnumType>, bool> = true>
#define mc_enable_if_is_json_(JsonType) template<typename JsonType, std::enable_if_t<is_maf_compatible_json<JsonType>::value, bool> = true>


namespace maf {
namespace srz {

using namespace nstl;

inline std::string getIndent(int indentLevel, bool newLine = false)
{
    if(newLine)
    {
        auto str = std::string(static_cast<size_t>(indentLevel * 4) + 1, ' ');
        str[0] = '\n';
        return str;
    }
    else
    {
        return std::string(static_cast<size_t>(indentLevel * 4), ' ');
    }
}

struct hlp
{
    template<typename T, std::enable_if_t<nstl::is_number_type_v<T>, bool> = true>
    inline static std::string quote(T value) { return quote(std::to_string(value)); }
    inline static std::string quote(const std::string& str) { return '"' + str + '"'; }
};

template <class JsonClass>
struct JsonTrait;

template<typename NonDeterminedType, typename = void>
struct DumpHelper
{
    inline static void dump(const NonDeterminedType& value, int indentLevel, std::string& strOut) noexcept
    {
        return prv::template dump<NonDeterminedType>(value, indentLevel, strOut);
    }

    struct prv
    {
        mc_enable_if_is_tuplelike_(TupleLike)
            inline static void dump(const TupleLike& value, int indentLevel, std::string& strOut) noexcept
        {
            value.dump(indentLevel, strOut);
        }

        mc_enable_if_is_number_(NumberType)
            inline static void dump(const NumberType& value, int /*indentLevel*/, std::string& strOut) noexcept
        {
            strOut += std::to_string(value);
        }

        mc_enable_if_is_enum_(EnumType)
            inline static void dump(const EnumType& value, int /*indentLevel*/, std::string& strOut) noexcept
        {
            strOut += std::to_string(static_cast<uint32_t>(value));
        }

        mc_enable_if_is_ptr_(PointerType)
            inline static void dump(const PointerType& value, int indentLevel, std::string& strOut) noexcept
        {
            if(value)
            {
                using NormalTypeOfPointerType = std::remove_const_t<std::remove_pointer_t<PointerType>>;
                DumpHelper<NormalTypeOfPointerType>::dump(*value, indentLevel, strOut);
            }
        }

        mc_enable_if_is_smartptr_(SmartPtrType)
            inline static void dump(const SmartPtrType& value, int indentLevel, std::string& strOut) noexcept
        {
            using PtrType = typename SmartPtrType::element_type*;
            DumpHelper<PtrType>::dump(value.get(), indentLevel, strOut);
        }

        mc_enable_if_is_json_(JsonType)
            inline static void dump(const JsonType& value, int /*indentLevel*/, std::string& strOut) noexcept
        {
            strOut += JsonTrait<JsonType>::marshall(value);
        }
    };
};

template <class JsonClass>
struct DumpHelper<JsonTrait<JsonClass>, void>
{
    inline static void dump(const bool& value, int /*indentLevel*/, std::string& strOut) noexcept
    {
        strOut += value ? "true" : "false";
    }
};

template <>
struct DumpHelper<bool, void>
{
    inline static void dump(const bool& value, int /*indentLevel*/, std::string& strOut) noexcept
    {
        strOut += value ? "true" : "false";
    }
};

template <typename T1, typename T2>
struct DumpHelper<std::pair<T1, T2>, void>
{
    using DType = std::pair<T1, T2>;

    inline static void dump(const DType& p, int indentLevel, std::string& strOut) noexcept
    {
        DumpHelper<pure_type_t<decltype(p.first)>>::dump(p.first, indentLevel, strOut);
        strOut += " : ";
        DumpHelper<pure_type_t<decltype(p.second)>>::dump(p.second , indentLevel, strOut);
    }
};

template<typename Tuple>
struct DumpHelper<Tuple, std::enable_if_t<nstl::is_tuple_v<Tuple>, void>>
{
    inline static void dump(const Tuple& tp, int indentLevel, std::string& strOut) noexcept {
        constexpr bool newLine = true;
        strOut += "[";
        nstl::tuple_for_each(tp, [indentLevel, &strOut, &newLine](const auto& elem) {
            strOut += getIndent(indentLevel + 1, newLine);
            DumpHelper<pure_type_t<decltype(elem)>>::dump(elem, indentLevel + 1, strOut);
            strOut += ",";
        });

        strOut.resize(strOut.size() - 1); // remove the last ',' character
        strOut += getIndent(indentLevel, newLine) + "]";
    }
};

template<>
struct DumpHelper<std::string, void>
{
    inline static void dump(const std::string& value, int /*indentLevel*/, std::string& strOut) noexcept {
        //must be taken care for case of wstring
        strOut += hlp::quote(value);
    }
};

template<typename StringDerived>
struct DumpHelper<StringDerived,
                  std::enable_if_t<std::is_base_of_v<std::string, StringDerived>, void>>
{

    inline static void dump(const StringDerived& value, int /*indentLevel*/, std::string& strOut) noexcept {
        //must be taken care for case of wstring
        strOut += hlp::quote(static_cast<const std::string&>(value));
    }
};

template<>
struct DumpHelper<const char*, void>
{
    inline static void dump(const char* value, int /*indentLevel*/, std::string& strOut) noexcept {
        //must be taken care for case of wstring
        strOut += hlp::quote(value);
    }
};

template<>
struct DumpHelper<std::wstring, void>
{

    inline static void dump(const std::wstring& /*value*/, int /*indentLevel*/, std::string& strOut) noexcept {
        strOut += "??? please enable logging wstring mode???";
    }
};

template<typename Iterable, typename = void>
struct Packager
{
    inline static void openBox(int /*indentLevel*/, std::string& strOut)
    {
        strOut += " [";
    }
    inline static void closeBox(int indentLevel, std::string& strOut)
    {
        strOut += getIndent(indentLevel, true) + "]";
    }
};

template<typename AssociateContainer>
struct Packager<AssociateContainer,
                nstl::to_void<typename AssociateContainer::key_type, typename AssociateContainer::mapped_type>>
{
    inline static void openBox(int /*indentLevel*/, std::string& strOut)
    {
        strOut += "{";
    }
    inline static void closeBox(int indentLevel, std::string& strOut)
    {
        strOut += getIndent(indentLevel, true) + "}";
    }
};


template<class Container>
struct DumpHelper<Container, std::enable_if_t<nstl::is_iterable_v<Container>, void>>
{
    inline static void dump(const Container& seq, int indentLevel, std::string& strOut) noexcept
    {
        constexpr bool NEWLINE = true;
        auto elemPreSeparator = getIndent(indentLevel + 1, NEWLINE);
        Packager<Container>::openBox(indentLevel, strOut);
        for (const auto& elem : seq) {
            strOut += elemPreSeparator;
            DumpHelper<typename Container::value_type>::dump(elem, indentLevel + 1, strOut);
            strOut += ",";
        }
        if(!seq.empty())
        {
            strOut.resize(strOut.size() - 1); // remove the last ',' character
        }
        Packager<Container>::closeBox(indentLevel, strOut);
    }
};

template <typename T>
void dump(const T& val, int indentLevel, std::string& strOut) { DumpHelper<std::decay_t<decltype (val)>>::dump(val, indentLevel, strOut); }

}// srz
}// maf

#undef mc_enable_if_is_tuplelike_
#undef mc_enable_if_is_number_
#undef mc_enable_if_is_smartptr_
#undef mc_enable_if_is_ptr_
#undef mc_enable_if_is_enum_
#undef mc_enable_if_is_json_
