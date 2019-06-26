#ifndef DUMPHELPER_H
#define DUMPHELPER_H

#include "thaf/Utils/CppExtension/TupleManip.h"
#include "thaf/Utils/CppExtension/TypeTraits.h"
#include "ByteArray.h"
#include <functional>
#include <tuple>
#include <string>
#include <cstring>
#include <vector>
#include <set>
#include <list>
#include <unordered_set>
#include <unordered_map>
#include <map>
#include <cassert>
#include <locale>

namespace thaf {
namespace srz {


#define PURE_TYPE(value) typename std::decay<decltype(value)>::type
#define PURE_TYPE_OF_TYPE(Type) typename std::decay<Type>::type


inline static std::string getIndent(int indentLevel, bool newLine = false)
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
    template<typename T, std::enable_if_t<stl::is_number_type_v<T>, bool> = true>
    static std::string quote(T value) { return quote(std::to_string(value)); }
    static std::string quote(const std::string& str) { return '"' + str + '"'; }
};

template<typename NonDeterminedType>
struct DumpHelper
{
    inline static void dump(const NonDeterminedType& value, int level, std::string& strOut) noexcept
    {
        return prv::template dump<NonDeterminedType>(value, level, strOut);
    }

    struct prv
    {

#define mc_enable_if_is_tuplewrap_(TypeName) template<typename TypeName, std::enable_if_t<stl::is_tuple_v<typename TypeName::value_type>, bool> = true>
#define mc_enable_if_is_number_(NumberType) template<typename NumberType, std::enable_if_t<stl::is_number_type_v<NumberType> , bool> = true>
#define mc_enable_if_is_smartptr_(SmartPtrType) template<typename SmartPtrType, std::enable_if_t<stl::is_smart_ptr_v<SmartPtrType>, bool> = true>
#define mc_enable_if_is_ptr_(PointerType) template<typename PointerType, std::enable_if_t<std::is_pointer_v<PointerType>, bool> = true>
#define mc_enable_if_is_enum_(EnumType) template<typename EnumType, std::enable_if_t<std::is_enum_v<EnumType>, bool> = true>

        mc_enable_if_is_tuplewrap_(TupleWrap)
            inline static void dump(const TupleWrap& value, int level, std::string& strOut) noexcept
        {
            value.dump(level, strOut);
        }

        mc_enable_if_is_number_(NumberType)
            inline static void dump(const NumberType& value, int /*level*/, std::string& strOut) noexcept
        {
            strOut += std::to_string(value);
        }

        mc_enable_if_is_enum_(EnumType)
            inline static void dump(const EnumType& value, int /*level*/, std::string& strOut) noexcept
        {
            strOut += std::to_string(static_cast<uint32_t>(value));
        }

        mc_enable_if_is_ptr_(PointerType)
            inline static void dump(const PointerType& value, int level, std::string& strOut) noexcept
        {
            if(value)
            {
                using NormalTypeOfPointerType = std::remove_const_t<std::remove_pointer_t<PointerType>>;
                DumpHelper<NormalTypeOfPointerType>::dump(*value, level, strOut);
            }
        }

        mc_enable_if_is_smartptr_(SmartPtrType)
            inline static void dump(const SmartPtrType& value, int level, std::string& strOut) noexcept
        {
            using PtrType = typename SmartPtrType::element_type*;
            return DumpHelper<PtrType>::dump(value.get(), level, strOut);
        }

    };

#undef mc_enable_if_is_tuplewrap_
#undef mc_enable_if_is_number_
#undef mc_enable_if_is_smartptr_
#undef mc_enable_if_is_ptr_
#undef mc_enable_if_is_enum_

};

template <>
struct DumpHelper<bool>
{
    inline static void dump(const bool& value, int /*level*/, std::string& strOut) noexcept
    {
        strOut += value ? "true" : "false";
    }
};

template <typename T1, typename T2>
struct DumpHelper<std::pair<T1, T2>>
{
    using DType = std::pair<T1, T2>;

    inline static void dump(const DType& p, int level, std::string& strOut) noexcept
    {
        DumpHelper<PURE_TYPE(p.first)>::dump(p.first, level, strOut);
        strOut += " : ";
        DumpHelper<PURE_TYPE(p.second)>::dump(p.second , level, strOut);
    }
};

template<typename ... ElemType >
struct DumpHelper<std::tuple<ElemType...> >
{
    using Tuple = std::tuple<ElemType...>;

    inline static void dump(const Tuple& tp, int level, std::string& strOut) noexcept {
        constexpr bool newLine = true;
        strOut += "<<";
        stl::tuple_for_each(tp, [level, &strOut, &newLine](const auto& elem) {
            strOut += getIndent(level + 1, newLine);
            DumpHelper<PURE_TYPE(elem)>::dump(elem, level + 1, strOut);
        });
        strOut += getIndent(level, newLine) + " >>";
    }
};

template<>
struct DumpHelper<std::string>
{

    inline static void dump(const std::string& value, int /*level*/, std::string& strOut) noexcept {
        //must be taken care for case of wstring
        strOut += hlp::quote(value);
    }
};
template<>
struct DumpHelper<ByteArray>
{

    inline static void dump(const ByteArray& value, int /*level*/, std::string& strOut) noexcept {
        //must be taken care for case of wstring
        strOut += hlp::quote(static_cast<const std::string&>(value));
    }
};

template<>
struct DumpHelper<const char*>
{
    inline static void dump(const char* value, int /*level*/, std::string& strOut) noexcept {
        //must be taken care for case of wstring
        strOut += hlp::quote(value);
    }
};

template<>
struct DumpHelper<std::wstring>
{

    inline static void dump(const std::wstring& /*value*/, int /*level*/, std::string& strOut) noexcept {
        strOut += "??? please enable logging wstring mode???";
    }
};

struct SequencePackager
{
    inline static void openBox(int /*level*/, std::string& strOut)
    {
        strOut += " [";
    }
    inline static void closeBox(int level, std::string& strOut)
    {
        strOut += getIndent(level, true) + "]";
    }
};

struct AssociativePackager
{
    inline static void openBox(int /*level*/, std::string& strOut)
    {
        strOut += "{";
    }
    inline static void closeBox(int level, std::string& strOut)
    {
        strOut += getIndent(level, true) + "}";
    }
};


template<class Container, class Packager>
struct ContainerDH
{
    inline static void dump(const Container& seq, int level, std::string& strOut) noexcept
    {
        constexpr bool newLine = true;
        Packager::openBox(level, strOut);
        for (const auto& elem : seq) {
            strOut += getIndent(level + 1, newLine);
            DumpHelper<typename Container::value_type>::dump(elem, level + 1, strOut);
            strOut += ",";
        }
        if(!seq.empty())
        {
            strOut.resize(strOut.size() - 1); // remove the last ',' character
        }
        Packager::closeBox(level, strOut);
    }
};



template<class Containter>
struct SequenceDH : public ContainerDH<Containter, SequencePackager>{};
template<class Containter>
struct AssociativeDH : public ContainerDH<Containter, AssociativePackager>{};

#define SPECIALIZE_SEQUENCE_DumpHelper(Container) template<typename ElemType > struct DumpHelper< Container<ElemType> > : public SequenceDH< Container<ElemType> >{};
#define SPECIALIZE_ASSOCIATIVE_DumpHelper(Container) template<typename Key, typename Value > struct DumpHelper< Container<Key, Value> > : public AssociativeDH< Container<Key, Value> >{};

SPECIALIZE_SEQUENCE_DumpHelper(std::vector)
SPECIALIZE_SEQUENCE_DumpHelper(std::set)
SPECIALIZE_SEQUENCE_DumpHelper(std::multiset)
SPECIALIZE_SEQUENCE_DumpHelper(std::unordered_set)
SPECIALIZE_SEQUENCE_DumpHelper(std::list)
SPECIALIZE_ASSOCIATIVE_DumpHelper(std::map)
SPECIALIZE_ASSOCIATIVE_DumpHelper(std::unordered_map)
SPECIALIZE_ASSOCIATIVE_DumpHelper(std::multimap)




}// srz
}// thaf

#endif // DUMPHELPER_H
