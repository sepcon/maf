#pragma once

#include "thaf/utils/cppextension/TupleManip.h"
#include "thaf/utils/cppextension/TypeTraits.h"
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

namespace thaf {
namespace srz {

/**
 * @brief SizeType represent container's size type:
 * In the effort to make compatible between 32 and 64 bit applications, the size of container types
 * is always declared as size_t with the widths(4byte/32bit - 8byte/64bit) differ on different
 * application types then it makes data serialized by 32bit apps cannot be read by 64bit apps.
 * Then by force use of SizeType as uint32_t the issue can be overcome by declaring variables to be serialized as specific type like
 * uint8_t, int8_t, uint16_t, uint32_t...
 */
using SizeType = uint32_t;
constexpr SizeType SIZETYPE_WIDE = sizeof(SizeType);

using RequestMoreBytesCallback = std::function<void(const char** startp, const char** lastp, SizeType neededBytes)>;

#define PURE_TYPE(value) typename std::decay<decltype(value)>::type
#define PURE_TYPE_OF_TYPE(Type) typename std::decay<Type>::type

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


/**
 * Basic utility for serialization framework, that provide 3 main functionalities:
 * 1. Calculating size of object to be serialized
 * 2. Serialize the object in to byte_array
 * 3. Deserialize to re-structure the object from byte_array
 */
template<typename T>
struct SerializationTrait
{
    /**
     * @brief serializeSizeOf: Calculating size of object to be serialized
     * @param value: the const reference to object tobe serialized
     * @return number of contiguous bytes needed to serialize the provided object
     */
    inline static SizeType serializeSizeOf(const T& value) noexcept
    {
        return prv::template serializeSizeOf<T>(value);
    }

    /**
     * @brief serialize: Serialize the object in to byte_array
     * @param startp: the pointer to first character in byte_array to store the serialized object
     * @param value: the const reference to object tobe serialized
     * @return number of contiguous bytes used to store the object == serializeSizeOf(value)
     */
    inline static SizeType serialize(char* startp, const T& value) noexcept
    {
        return prv::template serialize<T>(startp, value);
    }
    /**
     * @brief deserialize: re-structure the object from byte_array, thows the std::length_error if not enough bytes for re-constructing object
     * @param startp: pointer to first character to start read the bytes, the value of this pointer will be changed to point
     * to next byte after the last byte used for deserializing the object
     * @param lastp: pointer to last character available in byte_array
     * @param requestMoreBytes: the callback function to request more bytes if the size of serialized object is
     * greater than size ofprovided byte_array
     * @return the deseralized object
     */
    inline static T deserialize(const char** startp, const char** lastp, RequestMoreBytesCallback requestMoreBytes = nullptr)
    {
        return prv::template deserialize<T>(startp, lastp, requestMoreBytes);
    }

    struct prv
    {

#define mc_enable_if_is_tuplewrap_(TypeName) template<typename TypeName, std::enable_if_t<stl::is_tuple_v<typename TypeName::value_type>, bool> = true>
#define mc_enable_if_is_number_or_enum_(NumberType) template<typename NumberType, std::enable_if_t<stl::is_number_type<NumberType>::value || std::is_enum_v<NumberType>, bool> = true>
#define mc_enable_if_is_smartptr_(SmartPtrType) template<typename SmartPtrType, std::enable_if_t<stl::is_smart_ptr_v<SmartPtrType>, bool> = true>
#define mc_enable_if_is_ptr_(PointerType) template<typename PointerType, std::enable_if_t<std::is_pointer_v<PointerType>, bool> = true>
#define mc_enable_if_is_a_char_string(CharString) template <typename CharString, std::enable_if_t<std::is_base_of_v<std::string, CharString> && !std::is_same_v<std::string, CharString>, bool> = true>
#define mc_must_default_constructible(PointerType) static_assert (std::is_default_constructible_v<std::remove_pointer_t<PointerType> >, "");


        mc_enable_if_is_tuplewrap_(TupleWrap)
            inline static SizeType serializeSizeOf(const TupleWrap& value) noexcept
        {
            return SerializationTrait<typename TupleWrap::value_type>::serializeSizeOf(value._data);
        }

        mc_enable_if_is_tuplewrap_(TupleWrap)
            inline static SizeType serialize(char* startp, const TupleWrap& value) noexcept
        {
            return SerializationTrait<typename TupleWrap::value_type>::serialize(startp, value._data);
        }

        mc_enable_if_is_tuplewrap_(TupleWrap)
            inline static TupleWrap deserialize(const char** startp, const char** lastp, RequestMoreBytesCallback requestMoreBytes = nullptr)
        {
            TupleWrap value;
            value._data = SerializationTrait<typename TupleWrap::value_type>::deserialize(startp, lastp, requestMoreBytes);
            return value;
        }

        mc_enable_if_is_number_or_enum_(NumberOrEnum)
        inline static SizeType serializeSizeOf(const NumberOrEnum& /*value*/) noexcept
        {
            return sizeof(NumberOrEnum);
        }

        mc_enable_if_is_number_or_enum_(NumberOrEnum)
        inline static SizeType serialize(char* startp, const NumberOrEnum& value) noexcept
        {
            auto serializedCount = serializeSizeOf(value);
            *reinterpret_cast<PURE_TYPE_OF_TYPE(NumberOrEnum)*>(startp) = value;
            return serializedCount;
        }
        mc_enable_if_is_number_or_enum_(NumberOrEnum)
            inline static NumberOrEnum deserialize(const char** startp, const char** lastp, RequestMoreBytesCallback requestMoreBytes = nullptr)
        {
            makeSureDeserializable(startp, lastp, sizeof(NumberOrEnum), requestMoreBytes);
            auto value = *(reinterpret_cast<const PURE_TYPE_OF_TYPE(NumberOrEnum)*>(*startp));
            *startp += serializeSizeOf(value);
            return value;
        }

        mc_enable_if_is_ptr_(PointerType)
        inline static SizeType serializeSizeOf(PointerType value) noexcept
        {
            SizeType size = 1;
            if(value)
            {
                using NormalTypeOfPointerType = std::remove_const_t<std::remove_pointer_t<PointerType>>;
                size += SerializationTrait<NormalTypeOfPointerType>::serializeSizeOf(*value);
            }
            return size;
        }

        mc_enable_if_is_ptr_(PointerType)
        inline static SizeType serialize(char* startp, PointerType value) noexcept
        {
            //Won't serialize type that is not default_constructible because later won't be able to create object of that types to deserialize
            mc_must_default_constructible(PointerType)
            SizeType bytesUsed = 1;
            if(value)
            {
                using NormalTypeOfPointerType = std::remove_const_t<std::remove_pointer_t<PointerType>>;
                *startp = 1;
                bytesUsed += SerializationTrait<NormalTypeOfPointerType>::serialize(startp + 1, *value);
            }
            else
            {
                *startp = 0;
            }
            return bytesUsed;
        }

        mc_enable_if_is_ptr_(PointerType)
        inline static PointerType deserialize(const char** startp, const char** lastp, RequestMoreBytesCallback requestMoreBytes = nullptr)
        {
            mc_must_default_constructible(PointerType);
            using NormalTypeOfPointerType = std::remove_const_t<std::remove_pointer_t<PointerType>>;

            makeSureDeserializable(startp, lastp, 1, requestMoreBytes);
            bool isNotNull = static_cast<bool>(**startp);
            *startp += 1;
            NormalTypeOfPointerType* value = nullptr;
            if(isNotNull)
            {
                value = new NormalTypeOfPointerType;
                *value = SerializationTrait<NormalTypeOfPointerType>::deserialize(startp, lastp, requestMoreBytes);
            }
            return value;
        }

        mc_enable_if_is_smartptr_(SmartPtrType)
            inline static SizeType serializeSizeOf(const SmartPtrType& value) noexcept
        {
            using PtrType = typename SmartPtrType::element_type*;
            return SerializationTrait<PtrType>::serializeSizeOf(value.get());
        }

        mc_enable_if_is_smartptr_(SmartPtrType)
            inline static SizeType serialize(char* startp, const SmartPtrType& value) noexcept
        {
            using PtrType = typename SmartPtrType::element_type*;
            return SerializationTrait<PtrType>::serialize(startp, value.get());
        }

        mc_enable_if_is_smartptr_(SmartPtrType)
        inline static SmartPtrType deserialize(const char** startp, const char** lastp, RequestMoreBytesCallback requestMoreBytes = nullptr)
        {
            using PtrType = typename SmartPtrType::element_type*;
            return SmartPtrType(SerializationTrait<PtrType>::deserialize(startp, lastp, requestMoreBytes));
        }
    };

#undef mc_enable_if_is_tuplewrap_
#undef mc_enable_if_is_number_or_enum_
#undef mc_enable_if_is_smartptr_
#undef mc_enable_if_is_ptr_
#undef mc_must_default_constructible_

};

template <typename T1, typename T2>
struct SerializationTrait<std::pair<T1, T2>>
{
    using DType = std::pair<T1, T2>;
    inline static SizeType serializeSizeOf(const DType& p)  noexcept
    {
        return SerializationTrait<PURE_TYPE(p.first)>::serializeSizeOf(p.first) +
            SerializationTrait<PURE_TYPE(p.second)>::serializeSizeOf(p.second);
    }

    inline static SizeType serialize(char* startp, const DType& p) noexcept
    {
        SizeType bytesCount = 0;
        bytesCount += SerializationTrait<PURE_TYPE(p.first)>::serialize(startp, p.first);
        bytesCount += SerializationTrait<PURE_TYPE(p.second)>::serialize(startp + bytesCount, p.second);
        return bytesCount;
    }
    inline static DType deserialize(const char** startp, const char**  lastp, RequestMoreBytesCallback requestMoreBytes = nullptr)
    {
        DType p;
        p.first = SerializationTrait<PURE_TYPE(p.first)>::deserialize(startp, lastp, requestMoreBytes);
        p.second = SerializationTrait<PURE_TYPE(p.second)>::deserialize(startp, lastp, requestMoreBytes);
        return p;
    }
};

template<typename ... ElemType >
struct SerializationTrait<std::tuple<ElemType...> >
{
    using SizeTypeSerializer = SerializationTrait<SizeType>;

    inline static SizeType serializeSizeOf(const std::tuple<ElemType...>& tp) noexcept {
        SizeType contentSize = 0;
        stl::tuple_for_each(tp, [&contentSize](const auto& elem) {
            contentSize += SerializationTrait<PURE_TYPE(elem)>::serializeSizeOf(elem);
        });
        return contentSize;
    }

    inline static SizeType serialize(char* startp, const std::tuple<ElemType...>& tp) noexcept {
        SizeType serializedCount = 0;
        stl::tuple_for_each(tp, [&serializedCount, &startp](const auto& elem) {
            serializedCount += SerializationTrait<PURE_TYPE(elem)>::serialize(startp + serializedCount, elem);
        });
        return serializedCount;
    }

    inline static std::tuple<ElemType...> deserialize(const char** startp, const char**  lastp, RequestMoreBytesCallback requestMoreBytes = nullptr) {
        std::tuple<ElemType...> tp;
        stl::tuple_for_each(tp, [&startp, &lastp, requestMoreBytes](auto& elem) {
            elem = SerializationTrait<PURE_TYPE(elem)>::deserialize(startp, lastp, requestMoreBytes);
        });
        return tp;
    }
};

template<typename CharT, class Trait, class Allocator>
struct SerializationTrait<std::basic_string<CharT, Trait, Allocator> >
{
    using SizeTypeSerializer = SerializationTrait<SizeType>;
    using ValueType = std::basic_string<CharT>;
    inline static SizeType contentSizeOf(const ValueType& s) noexcept
    {
        return static_cast<SizeType>( s.size() ) * sizeof (CharT);
    }
    inline static SizeType serializeSizeOf(const ValueType& value) noexcept {
        return SIZETYPE_WIDE + static_cast<SizeType>(contentSizeOf(value));
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

template<>
struct SerializationTrait<ByteArray>
{
    using SizeTypeSerializer = SerializationTrait<SizeType>;
    using ValueType = ByteArray;
    inline static SizeType contentSizeOf(const ValueType& s) noexcept
    {
        return static_cast<SizeType>( s.size() ) * sizeof (ValueType::value_type);
    }
    inline static SizeType serializeSizeOf(const ValueType& value) noexcept {
        return SIZETYPE_WIDE + static_cast<SizeType>(contentSizeOf(value));
    }
    inline static SizeType serialize(char* startp, const ValueType& value) noexcept {
        return SerializationTrait<std::string>::serialize(startp, static_cast<const std::string&>(value));
    }
    inline static ValueType deserialize(const char** startp, const char** lastp, RequestMoreBytesCallback requestMoreBytes = nullptr) {
        ValueType value;
        static_cast<std::string&>(value) = SerializationTrait<std::string>::deserialize(startp, lastp, requestMoreBytes);
        return value;
    }
};

template <class Container>
struct Reserver
{
    static void reserve(Container& /*c*/, SizeType /*size*/){}
};

template<typename T>
struct Reserver<std::vector<T>>
{
    static void reserve(std::vector<T>& c, SizeType size){ c.reserve(static_cast<size_t>(size)); }
};

template<class Sequence>
struct SequenceSC
{
    inline static SizeType serializeSizeOf(const Sequence& c) noexcept
    {
        SizeType contentSize = 0;
        for (const auto& e : c) {
            contentSize += SerializationTrait<typename Sequence::value_type>::serializeSizeOf(e);
        }
        return SIZETYPE_WIDE + contentSize;
    }
};
template <class Sequence>
struct SequenceSz
{
    inline static SizeType serialize(char* startp, const Sequence& c) noexcept
    {
        SizeType serializedCount = 0;
        SizeType numberOfElems = static_cast<SizeType>(c.size());
        serializedCount += SerializationTrait<SizeType>::serialize(startp, numberOfElems);
        for (const auto& elem : c) {
            serializedCount += SerializationTrait<typename Sequence::value_type>::serialize(startp + serializedCount, elem);
        }
        return serializedCount;
    }
};
template <class Sequence>
struct SequenceDsz
{
    using SizeTypeSerializer = SerializationTrait<SizeType>;
    using ElemSerializer = SerializationTrait<typename Sequence::value_type>;
    inline static Sequence deserialize(const char** startp, const char**  lastp, RequestMoreBytesCallback requestMoreBytes = nullptr) {
        Sequence c;
        auto size = SizeTypeSerializer::deserialize(startp, lastp, requestMoreBytes);
        if (size > 0) {
            Reserver<Sequence>::reserve(c, size);
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
        SizeType size = SerializationTrait<SizeType>::deserialize(startp, lastp, requestMoreBytes);
        for (SizeType i = 0; i < size; ++i)
        {
            auto key = SerializationTrait<PURE_TYPE_OF_TYPE(KeyType)>::deserialize(startp, lastp, requestMoreBytes);
            auto value = SerializationTrait<PURE_TYPE_OF_TYPE(ValueType)>::deserialize(startp, lastp, requestMoreBytes);
            m.emplace(std::move(key), std::move(value));
        }
        return m;
    }
};

template<class Containter>
struct SequenceSerializer : public SequenceSC<Containter>, public SequenceSz<Containter>, public SequenceDsz<Containter> {};
template<class Containter>
struct AssociativeSerializer : public SequenceSC<Containter>, public SequenceSz<Containter>, public AssociativeDsz<Containter> {};

#define SPECIALIZE_SEQUENCE_SERIALIZATION(Container) template<typename ElemType > struct SerializationTrait< Container<ElemType> > : public SequenceSerializer< Container<ElemType> >{};
#define SPECIALIZE_ASSOCIATIVE_SERIALIZATION(Container) template<typename Key, typename Value > struct SerializationTrait< Container<Key, Value> > : public AssociativeSerializer< Container<Key, Value> >{};

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
