#pragma once

#include "headers/Libs/Utils/STLExtension/TupleManip.h"
#include "headers/Libs/Utils/STLExtension/TypeTraits.h"
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

namespace thaf {
namespace srz {

using SizeType = size_t;
using RequestMoreBytesCallback = std::function<void(const char** startp, const char** lastp, SizeType neededBytes)>;

#define PURE_TYPE(value) typename std::decay<decltype(value)>::type
#define PURE_TYPE_OF_TYPE(Type) typename std::decay<Type>::type

inline constexpr SizeType SIZETYPE_WIDE = sizeof(SizeType);

inline SizeType byteCountInRangeOf(const char* const first, const char* const last)
{
    if ((first && last) && (last >= first))
    {
        return static_cast<SizeType>(last - first + 1) * sizeof(char);
    }
    else
    {
        return 0;
    }
}

inline void makeSureDeserializable(const char** startp, const char** lastp, SizeType neededBytes, RequestMoreBytesCallback requestMoreBytes)
{
    if(byteCountInRangeOf(*startp, *lastp) < neededBytes)
    {
        if(requestMoreBytes != nullptr)
        {
            requestMoreBytes(startp, lastp, neededBytes);
            if(byteCountInRangeOf(*startp, *lastp) >= neededBytes)
            {
                return; //serializable
            }
        }
        throw std::runtime_error("Not enough bytes for deserilization");
    }
    else
    {
        //serializable
    }
}


template<typename NonDeterminedType>
struct Serialization
{
    inline static SizeType serializeSizeOf(const NonDeterminedType& value) noexcept
    {
        return prv::template serializeSizeOf<NonDeterminedType>(value);
    }

    inline static SizeType serialize(char* startp, const NonDeterminedType& value) noexcept
    {
        return prv::template serialize<NonDeterminedType>(startp, value);
    }
    inline static NonDeterminedType deserialize(const char** startp, const char** lastp, RequestMoreBytesCallback requestMoreBytes = nullptr)
    {
        return prv::template deserialize<NonDeterminedType>(startp, lastp, requestMoreBytes);
    }

    struct prv
    {
        template<typename TupleWrap, std::enable_if_t<stl::is_tuple_v<typename TupleWrap::value_type>, bool> = true>
        inline static SizeType serializeSizeOf(const TupleWrap& value) noexcept
        {
            return Serialization<typename TupleWrap::value_type>::serializeSizeOf(value._data);
        }

        template<typename TupleWrap, std::enable_if_t<stl::is_tuple_v<typename TupleWrap::value_type>, bool> = true>
        inline static SizeType serialize(char* startp, const TupleWrap& value) noexcept
        {
            return Serialization<typename TupleWrap::value_type>::serialize(startp, value._data);
        }

        template<typename TupleWrap, std::enable_if_t<stl::is_tuple_v<typename TupleWrap::value_type>, bool> = true>
        inline static TupleWrap deserialize(const char** startp, const char** lastp, RequestMoreBytesCallback requestMoreBytes = nullptr)
        {
            TupleWrap value;
            value._data = Serialization<typename TupleWrap::value_type>::deserialize(startp, lastp, requestMoreBytes);
            return value;
        }

        template<typename NumberType, std::enable_if_t<stl::is_number_type<NumberType>::value, bool> = true>
        inline static SizeType serializeSizeOf(const NumberType& /*value*/) noexcept
        {
            static_assert (stl::is_number_type<NumberType>::value, "Not a number type");
            return sizeof(NumberType);
        }

        template<typename NumberType, std::enable_if_t<stl::is_number_type<NumberType>::value, bool> = true>
        inline static SizeType serialize(char* startp, const NumberType& value) noexcept
        {
            static_assert (stl::is_number_type<NumberType>::value, "Not a number type");
            auto serializedCount = serializeSizeOf(value);
            *reinterpret_cast<PURE_TYPE_OF_TYPE(NumberType)*>(startp) = value;
            return serializedCount;
        }
        template<typename NumberType, std::enable_if_t<stl::is_number_type<NumberType>::value, bool> = true>
        inline static NumberType deserialize(const char** startp, const char** lastp, RequestMoreBytesCallback requestMoreBytes = nullptr)
        {
            static_assert (stl::is_number_type<NumberType>::value, "Not a number type");
            makeSureDeserializable(startp, lastp, sizeof(NumberType), requestMoreBytes);
            auto value = *(reinterpret_cast<const PURE_TYPE_OF_TYPE(NumberType)*>(*startp));
            *startp += serializeSizeOf(value);
            return value;
        }
    };

};



template <typename T1, typename T2>
struct Serialization<std::pair<T1, T2>>
{
    using DType = std::pair<T1, T2>;
    inline static SizeType serializeSizeOf(const DType& p)  noexcept
    {
        return Serialization<PURE_TYPE(p.first)>::serializeSizeOf(p.first) +
            Serialization<PURE_TYPE(p.second)>::serializeSizeOf(p.second);
    }

    inline static SizeType serialize(char* startp, const DType& p) noexcept
    {
        SizeType bytesCount = 0;
        bytesCount += Serialization<PURE_TYPE(p.first)>::serialize(startp, p.first);
        bytesCount += Serialization<PURE_TYPE(p.second)>::serialize(startp + bytesCount, p.second);
        return bytesCount;
    }
    inline static DType deserialize(const char** startp, const char**  lastp, RequestMoreBytesCallback requestMoreBytes = nullptr)
    {
        DType p;
        p.first = Serialization<PURE_TYPE(p.first)>::deserialize(startp, lastp, requestMoreBytes);
        p.second = Serialization<PURE_TYPE(p.second)>::deserialize(startp, lastp, requestMoreBytes);
        return p;
    }
};

template<typename ... ElemType >
struct Serialization<std::tuple<ElemType...> >
{
    using SizeTypeSerializer = Serialization<SizeType>;

    inline static SizeType serializeSizeOf(const std::tuple<ElemType...>& tp) noexcept {
        SizeType contentSize = 0;
        stl::tuple_for_each(tp, [&contentSize](const auto& elem) {
            contentSize += Serialization<PURE_TYPE(elem)>::serializeSizeOf(elem);
        });
        return contentSize;
    }

    inline static SizeType serialize(char* startp, const std::tuple<ElemType...>& tp) noexcept {
        SizeType serializedCount = 0;
        stl::tuple_for_each(tp, [&serializedCount, &startp](const auto& elem) {
            serializedCount += Serialization<PURE_TYPE(elem)>::serialize(startp + serializedCount, elem);
        });
        return serializedCount;
    }

    inline static std::tuple<ElemType...> deserialize(const char** startp, const char**  lastp, RequestMoreBytesCallback requestMoreBytes = nullptr) {
        std::tuple<ElemType...> tp;
        stl::tuple_for_each(tp, [&startp, &lastp, requestMoreBytes](auto& elem) {
            elem = Serialization<PURE_TYPE(elem)>::deserialize(startp, lastp, requestMoreBytes);
        });
        return tp;
    }
};

template<typename CharT>
struct Serialization<std::basic_string<CharT>>
{
    using SizeTypeSerializer = Serialization<SizeType>;
    using ValueType = std::basic_string<CharT>;
    inline static SizeType contentSizeOf(const ValueType& s) noexcept
    {
        return s.size() * sizeof (CharT);
    }
    inline static SizeType serializeSizeOf(const ValueType& value) noexcept {
        return sizeof(typename ValueType::size_type) + static_cast<SizeType>(contentSizeOf(value));
    }
    inline static SizeType serialize(char* startp, const ValueType& value) noexcept {
        auto bytesInString = contentSizeOf(value);
        auto serializedCount = SizeTypeSerializer::serialize(startp, bytesInString);
        memcpy(startp + serializedCount, reinterpret_cast<const char*>(value.c_str()), bytesInString);
        return serializedCount + bytesInString;
    }
    inline static ValueType deserialize(const char** startp, const char** lastp, RequestMoreBytesCallback requestMoreBytes = nullptr) {
        ValueType value;
        auto size = SizeTypeSerializer::deserialize(startp, lastp, requestMoreBytes);
        if (size > 0) {
            makeSureDeserializable(startp, lastp, size, requestMoreBytes);
            value.resize(size/sizeof(CharT));
            memcpy(reinterpret_cast<char*>(&value[0]), *startp, size);
            *startp += size;
        }
        return value;
    }
};


template<class Sequence>
struct SequenceSC
{
    inline static SizeType serializeSizeOf(const Sequence& c) noexcept
    {
        SizeType contentSize = 0;
        for (const auto& e : c) {
            contentSize += Serialization<typename Sequence::value_type>::serializeSizeOf(e);
        }
        return sizeof(typename Sequence::size_type) + contentSize;
    }
};
template <class Sequence>
struct SequenceSz
{
    inline static SizeType serialize(char* startp, const Sequence& c) noexcept
    {
        SizeType serializedCount = 0;
        auto numberOfElems = c.size();
        serializedCount += Serialization<typename Sequence::size_type>::serialize(startp, numberOfElems);
        for (const auto& elem : c) {
            serializedCount += Serialization<typename Sequence::value_type>::serialize(startp + serializedCount, elem);
        }
        return serializedCount;
    }
};
template <class Sequence>
struct SequenceDsz
{
    using SizeTypeSerializer = Serialization<typename Sequence::size_type>;
    using ElemSerializer = Serialization<typename Sequence::value_type>;
    inline static Sequence deserialize(const char** startp, const char**  lastp, RequestMoreBytesCallback requestMoreBytes = nullptr) {
        Sequence c;
        auto size = SizeTypeSerializer::deserialize(startp, lastp, requestMoreBytes);
        if (size > 0) {
            for (SizeType i = 0; i < size; ++i) {
                c.insert(c.end(), ElemSerializer::deserialize(startp, lastp, requestMoreBytes));
            }
        }
        return c;
    }
};
template<class Associative>
struct AssociativeDsz
{
    using KeyType = typename Associative::key_type;
    using ValueType = typename Associative::mapped_type;
    inline static Associative deserialize(const char** startp, const char**  lastp, RequestMoreBytesCallback requestMoreBytes = nullptr) {
        Associative m;
        SizeType size = Serialization<SizeType>::deserialize(startp, lastp, requestMoreBytes);
        for (SizeType i = 0; i < size; ++i)
        {
            auto key = Serialization<PURE_TYPE_OF_TYPE(KeyType)>::deserialize(startp, lastp, requestMoreBytes);
            auto value = Serialization<PURE_TYPE_OF_TYPE(ValueType)>::deserialize(startp, lastp, requestMoreBytes);
            m.emplace(std::move(key), std::move(value));
        }
        return m;
    }
};

template<class Containter>
struct SequenceSerializer : public SequenceSC<Containter>, public SequenceSz<Containter>, public SequenceDsz<Containter> {};
template<class Containter>
struct AssociativeSerializer : public SequenceSC<Containter>, public SequenceSz<Containter>, public AssociativeDsz<Containter> {};

#define SPECIALIZE_SEQUENCE_SERIALIZATION(Container) template<typename ElemType > struct Serialization< Container<ElemType> > : public SequenceSerializer< Container<ElemType> >{};
#define SPECIALIZE_ASSOCIATIVE_SERIALIZATION(Container) template<typename Key, typename Value > struct Serialization< Container<Key, Value> > : public AssociativeSerializer< Container<Key, Value> >{};

SPECIALIZE_SEQUENCE_SERIALIZATION(std::vector)
SPECIALIZE_SEQUENCE_SERIALIZATION(std::set)
SPECIALIZE_SEQUENCE_SERIALIZATION(std::multiset)
SPECIALIZE_SEQUENCE_SERIALIZATION(std::unordered_set)
SPECIALIZE_SEQUENCE_SERIALIZATION(std::list)
SPECIALIZE_ASSOCIATIVE_SERIALIZATION(std::map)
SPECIALIZE_ASSOCIATIVE_SERIALIZATION(std::unordered_map)
SPECIALIZE_ASSOCIATIVE_SERIALIZATION(std::multimap)




}// srz
}// thaf
