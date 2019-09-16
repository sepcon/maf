#pragma once

#include "maf/utils/cppextension/TupleManip.h"
#include "maf/utils/cppextension/TypeTraits.h"
#include "BasicTypes.h"
#include "JsonTrait.h"
#include "ByteArray.h"
#include <functional>
#include <tuple>
#include <string>
#include <cstring>
#include <cassert>

#define mc_enable_if_is_tuplelike_(TypeName) template<typename TypeName, std::enable_if_t<is_tuple_like_v<TypeName>, bool> = true>
#define mc_enable_if_is_number_or_enum_(NumberType) template<typename NumberType, std::enable_if_t<nstl::is_number_type<NumberType>::value || std::is_enum_v<NumberType>, bool> = true>
#define mc_enable_if_is_smartptr_(SmartPtrType) template<typename SmartPtrType, std::enable_if_t<nstl::is_smart_ptr_v<SmartPtrType>, bool> = true>
#define mc_enable_if_is_ptr_(PointerType) template<typename PointerType, std::enable_if_t<std::is_pointer_v<PointerType>, bool> = true>
#define mc_enable_if_is_a_char_string(CharString) template <typename CharString, std::enable_if_t<std::is_base_of_v<std::string, CharString> && !std::is_same_v<std::string, CharString>, bool> = true>
#define mc_must_default_constructible(PointerType) static_assert (std::is_default_constructible_v<std::remove_pointer_t<PointerType> >, "");
#define mc_enable_if_is_json_(JsonType) template<typename JsonType, std::enable_if_t< is_maf_compatible_json<JsonType>::value , bool> = true>
#define mc_enable_if_is_iterable_(Container) template<typename Container, std::enable_if_t<nstl::is_iterable_v<Container>, bool> = true>


namespace maf {
namespace srz {

using       SizeType                    = uint32_t;
constexpr   SizeType SIZETYPE_WIDE      = sizeof(SizeType);
using       RequestMoreBytesCallback    = std::function<void(const char** startp, const char** lastp, SizeType neededBytes)>;


template <typename T>
SizeType serializeSizeOf(const T& p)  noexcept;
template <typename T>
SizeType serialize(char* startp, const T& p) noexcept;
template <typename T>
T deserialize(const char** startp, const char**  lastp, RequestMoreBytesCallback requestMoreBytes = nullptr);


namespace internal
{

inline SizeType byteCountInRangeOf(const char* const first, const char* const last);
inline void makeSureDeserializable(const char** startp, const char** lastp, SizeType neededBytes, RequestMoreBytesCallback requestMoreBytes);

template<typename JsonType, std::enable_if_t<std::is_class_v<JsonTrait<JsonType>>, bool> = true>
inline SizeType jsonSerializeSizeOf(const JsonType &value) noexcept;

template<typename JsonType, std::enable_if_t<std::is_class_v<JsonTrait<JsonType>>, bool> = true>
inline SizeType jsonSerialize(char *startp, const JsonType &value) noexcept;

template<typename JsonType, std::enable_if_t<std::is_class_v<JsonTrait<JsonType>>, bool> = true>
inline JsonType jsonDeserialize(const char **startp, const char **lastp, RequestMoreBytesCallback requestMoreBytes);

template <class Container, typename = void>
struct ContainerReserver
{
    static void reserve(Container& /*c*/, SizeType /*size*/){}
};

template<typename Container>
struct ContainerReserver<Container, nstl::to_void<decltype (std::declval<Container>().reserve(0))>>
{
    static void reserve(Container& c, SizeType size){ c.reserve(static_cast<size_t>(size)); }
};

}

using namespace nstl;


/**
 * Basic utility for serialization framework, that provide 3 main functionalities:
 * 1. Calculating size of object to be serialized
 * 2. Serialize the object in to byte_array
 * 3. Deserialize to re-structure the object from byte_array
 */
template<typename T, typename = void>
struct SerializationTrait
{
    /**
     * @brief serializeSizeOf: Calculating size of object to be serialized
     * @param value: the const reference to object tobe serialized
     * @return number of contiguous bytes needed to serialize the provided object
     */
    inline static SizeType serializeSizeOf(const T& value) noexcept
    {
        return Impl::template serializeSizeOf<T>(value);
    }

    /**
     * @brief serialize: Serialize the object in to byte_array
     * @param startp: the pointer to first character in byte_array to store the serialized object
     * @param value: the const reference to object tobe serialized
     * @return number of contiguous bytes used to store the object == serializeSizeOf(value)
     */
    inline static SizeType serialize(char* startp, const T& value) noexcept
    {
        return Impl::template serialize<T>(startp, value);
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
        return Impl::template deserialize<T>(startp, lastp, requestMoreBytes);
    }

    struct Impl
    {
        mc_enable_if_is_tuplelike_(TupleLike)
            inline static SizeType serializeSizeOf(const TupleLike& value) noexcept
        {
            return SerializationTrait<typename TupleLike::data_type>::serializeSizeOf(value._data);
        }

        mc_enable_if_is_tuplelike_(TupleLike)
            inline static SizeType serialize(char* startp, const TupleLike& value) noexcept
        {
            return SerializationTrait<typename TupleLike::data_type>::serialize(startp, value._data);
        }

        mc_enable_if_is_tuplelike_(TupleLike)
            inline static TupleLike deserialize(const char** startp, const char** lastp, RequestMoreBytesCallback requestMoreBytes = nullptr)
        {
            TupleLike value;
            value._data = SerializationTrait<typename TupleLike::data_type>::deserialize(startp, lastp, requestMoreBytes);
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
            *reinterpret_cast<pure_type_t<NumberOrEnum>*>(startp) = value;
            return serializedCount;
        }
        mc_enable_if_is_number_or_enum_(NumberOrEnum)
            inline static NumberOrEnum deserialize(const char** startp, const char** lastp, RequestMoreBytesCallback requestMoreBytes = nullptr)
        {
            internal::makeSureDeserializable(startp, lastp, sizeof(NumberOrEnum), requestMoreBytes);
            auto value = *(reinterpret_cast<const pure_type_t<NumberOrEnum>*>(*startp));
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
            mc_must_default_constructible(PointerType)
            using NormalTypeOfPointerType = std::remove_const_t<std::remove_pointer_t<PointerType>>;

            internal::makeSureDeserializable(startp, lastp, 1, requestMoreBytes);
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

        mc_enable_if_is_json_(JsonType)
            inline static SizeType serializeSizeOf(const JsonType& value) noexcept
        {
            return internal::jsonSerializeSizeOf<JsonType>(value);
        }

        mc_enable_if_is_json_(JsonType)
            inline static SizeType serialize(char* startp, const JsonType& value) noexcept
        {
            return internal::jsonSerialize<JsonType>(startp, value);
        }

        mc_enable_if_is_json_(JsonType)
            inline static JsonType deserialize(const char** startp, const char** lastp, RequestMoreBytesCallback requestMoreBytes = nullptr)
        {
            return internal::jsonDeserialize<JsonType>(startp, lastp, requestMoreBytes);
        }

        mc_enable_if_is_iterable_(Container)
            inline static SizeType serializeSizeOf(const Container& c) noexcept
        {
            SizeType contentSize = 0;
            for (const auto& e : c) {
                contentSize += SerializationTrait<typename Container::value_type>::serializeSizeOf(e);
            }
            return SIZETYPE_WIDE + contentSize;
        }
        mc_enable_if_is_iterable_(Container)
            inline static SizeType serialize(char* startp, const Container& c) noexcept
        {
            SizeType serializedCount = 0;
            SizeType numberOfElems = static_cast<SizeType>(c.size());
            serializedCount += SerializationTrait<SizeType>::serialize(startp, numberOfElems);
            for (const auto& elem : c) {
                serializedCount += SerializationTrait<typename Container::value_type>::serialize(startp + serializedCount, elem);
            }
            return serializedCount;
        }

        template<typename Container, std::enable_if_t<nstl::is_iterable_v<Container> && nstl::is_back_insertible_v<Container>, bool> = true>
        inline static Container deserialize(const char** startp, const char**  lastp, RequestMoreBytesCallback requestMoreBytes = nullptr) {
            using SizeTypeSerializer = SerializationTrait<SizeType>;
            using ElemSerializer = SerializationTrait<typename Container::value_type>;
            Container c;
            auto size = SizeTypeSerializer::deserialize(startp, lastp, requestMoreBytes);
            if (size > 0) {
                internal::ContainerReserver<Container>::reserve(c, size);
                for (SizeType i = 0; i < size; ++i) {
                    c.push_back(ElemSerializer::deserialize(startp, lastp, requestMoreBytes));
                }
            }
            return c;
        }

        template<typename Container, std::enable_if_t<nstl::is_iterable_v<Container> && nstl::is_position_independent_insertible_v<Container>, bool> = true>
        inline static Container deserialize(const char** startp, const char**  lastp, RequestMoreBytesCallback requestMoreBytes = nullptr) {
            using SizeTypeSerializer = SerializationTrait<SizeType>;
            using ElemSerializer = SerializationTrait<typename Container::value_type>;
            Container c;
            auto size = SizeTypeSerializer::deserialize(startp, lastp, requestMoreBytes);
            if (size > 0) {
                internal::ContainerReserver<Container>::reserve(c, size);
                for (SizeType i = 0; i < size; ++i) {
                    c.insert(ElemSerializer::deserialize(startp, lastp, requestMoreBytes));
                }
            }
            return c;
        }

    };

};

template <typename Pair>
struct SerializationTrait<Pair,
                          nstl::to_void<typename Pair::first_type, typename Pair::second_type>>
{
    using DType = Pair;
    using NCDType = std::pair<std::remove_const_t<typename DType::first_type>,
                              std::remove_const_t<typename DType::second_type>>;

    inline static SizeType serializeSizeOf(const DType& p)  noexcept
    {
        return SerializationTrait<pure_type_t<decltype(p.first)>>::serializeSizeOf(p.first) +
            SerializationTrait<pure_type_t<decltype(p.second)>>::serializeSizeOf(p.second);
    }

    inline static SizeType serialize(char* startp, const DType& p) noexcept
    {
        SizeType bytesCount = 0;
        bytesCount += SerializationTrait<pure_type_t<decltype(p.first)>>::serialize(startp, p.first);
        bytesCount += SerializationTrait<pure_type_t<decltype(p.second)>>::serialize(startp + bytesCount, p.second);
        return bytesCount;
    }
    inline static NCDType deserialize(const char** startp, const char**  lastp, RequestMoreBytesCallback requestMoreBytes = nullptr)
    {
        NCDType p;
        p.first = SerializationTrait<typename NCDType::first_type>::deserialize(startp, lastp, requestMoreBytes);
        p.second = SerializationTrait<typename NCDType::second_type>::deserialize(startp, lastp, requestMoreBytes);
        return p;
    }
};

template<typename Tuple>
struct SerializationTrait<Tuple, std::enable_if_t<nstl::is_tuple_v<Tuple>, void>>
{
    using SizeTypeSerializer = SerializationTrait<SizeType>;

    inline static SizeType serializeSizeOf(const Tuple& tp) noexcept {
        SizeType contentSize = 0;
        nstl::tuple_for_each(tp, [&contentSize](const auto& elem) {
            contentSize += SerializationTrait<pure_type_t<decltype(elem)>>::serializeSizeOf(elem);
        });
        return contentSize;
    }

    inline static SizeType serialize(char* startp, const Tuple& tp) noexcept {
        SizeType serializedCount = 0;
        nstl::tuple_for_each(tp, [&serializedCount, &startp](const auto& elem) {
            serializedCount += SerializationTrait<pure_type_t<decltype(elem)>>::serialize(startp + serializedCount, elem);
        });
        return serializedCount;
    }

    inline static Tuple deserialize(const char** startp, const char**  lastp, RequestMoreBytesCallback requestMoreBytes = nullptr) {
        Tuple tp;
        nstl::tuple_for_each(tp, [&startp, &lastp, requestMoreBytes](auto& elem) {
            elem = SerializationTrait<pure_type_t<decltype(elem)>>::deserialize(startp, lastp, requestMoreBytes);
        });
        return tp;
    }
};

template<typename CharT, class Trait, class Allocator>
struct SerializationTrait<std::basic_string<CharT, Trait, Allocator>, void>
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
            internal::makeSureDeserializable(startp, lastp, size, requestMoreBytes);
            value.resize(size/sizeof(CharT));
            memcpy(reinterpret_cast<char*>(&value[0]), *startp, size);
            *startp += size;
        }
        return value;
    }
};

template<typename StringDerived>
struct SerializationTrait<StringDerived,
                          std::enable_if_t
                          <
                              std::is_base_of_v<std::string, StringDerived>
                                && !std::is_same_v<std::string, StringDerived>,
                              void
                              >
                          >
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


template <typename T>
SizeType serializeSizeOf(const T& v)  noexcept
{
    return SerializationTrait<T>::serializeSizeOf(v);
}
template <typename T>
SizeType serialize(char* startp, const T& v) noexcept
{
    return SerializationTrait<T>::serialize(startp, v);
}
template <typename T>
T deserialize(const char** startp, const char**  lastp, RequestMoreBytesCallback requestMoreBytes)
{
    return SerializationTrait<T>::deserialize(startp, lastp, requestMoreBytes);
}


namespace internal
{
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

template<typename JsonType, std::enable_if_t<std::is_class_v<JsonTrait<JsonType>>, bool>>
inline SizeType jsonSerializeSizeOf(const JsonType &value) noexcept
{
    return static_cast<SizeType>(JsonTrait<JsonType>::marshallSize(value))
           +
           SerializationTrait<SizeType>::serializeSizeOf(sizeof(SizeType));
}

template<typename JsonType, std::enable_if_t<std::is_class_v<JsonTrait<JsonType>>, bool>>
inline SizeType jsonSerialize(char *startp, const JsonType &value) noexcept
{
    std::string str = JsonTrait<JsonType>::marshall(value);
    return SerializationTrait<std::string>::serialize(startp, str);
}

template<typename JsonType, std::enable_if_t<std::is_class_v<JsonTrait<JsonType>>, bool>>
inline JsonType jsonDeserialize(const char **startp, const char **lastp, RequestMoreBytesCallback requestMoreBytes)
{
    auto str = SerializationTrait<std::string>::deserialize(startp, lastp, requestMoreBytes);
    return JsonTrait<JsonType>::unmarshall(str);
}

}


}// srz
}// maf

#undef mc_enable_if_is_tuplelike_
#undef mc_enable_if_is_number_or_enum_
#undef mc_enable_if_is_smartptr_
#undef mc_enable_if_is_ptr_
#undef mc_enable_if_is_a_char_string
#undef mc_must_default_constructible
#undef mc_enable_if_is_json_
#undef mc_enable_if_is_iterable_
