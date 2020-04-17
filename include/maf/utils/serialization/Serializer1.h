#ifndef SERIALIZER1_H
#define SERIALIZER1_H

#include "BasicTypes.h"
#include "JsonTrait.h"
#include <cassert>
#include <maf/utils/cppextension/TupleManip.h>
#include <maf/utils/cppextension/TypeTraits.h>
#include <string>
#include <tuple>

/// Serialization

#define mc_enable_if_is_tuplelike_(TypeName)                                   \
  template <typename TypeName,                                                 \
            std::enable_if_t<is_tuple_like_v<TypeName>, bool> = true>
#define mc_enable_if_is_number_or_enum_(NumberType)                            \
  template <typename NumberType,                                               \
            std::enable_if_t<nstl::is_number_type<NumberType>::value ||        \
                                 std::is_enum_v<NumberType>,                   \
                             bool> = true>
#define mc_enable_if_is_smartptr_(SmartPtrType)                                \
  template <typename SmartPtrType,                                             \
            std::enable_if_t<nstl::is_smart_ptr_v<SmartPtrType>, bool> = true>
#define mc_enable_if_is_ptr_(PointerType)                                      \
  template <typename PointerType,                                              \
            std::enable_if_t<std::is_pointer_v<PointerType>, bool> = true>
#define mc_enable_if_is_a_char_string(CharString)                              \
  template <typename CharString,                                               \
            std::enable_if_t<std::is_base_of_v<std::string, CharString> &&     \
                                 !std::is_same_v<std::string, CharString>,     \
                             bool> = true>
#define mc_must_default_constructible(PointerType)                             \
  static_assert(                                                               \
      std::is_default_constructible_v<std::remove_pointer_t<PointerType>>,     \
      "");
#define mc_enable_if_is_json_(JsonType)                                        \
  template <typename JsonType,                                                 \
            std::enable_if_t<is_maf_compatible_json<JsonType>::value, bool> =  \
                true>
#define mc_enable_if_is_iterable_(Container)                                   \
  template <typename Container,                                                \
            std::enable_if_t<nstl::is_iterable_v<Container>, bool> = true>

namespace maf {
namespace srz {

using SizeType = uint32_t;
constexpr SizeType SIZETYPE_WIDE = sizeof(SizeType);

template <class OStream, typename T> void serialize(OStream &, const T &);

template <class IStream, typename T> T deserialize(IStream &, bool &);

namespace internal {

template <class OStream, class IStream, typename JsonType,
          std::enable_if_t<std::is_class_v<JsonTrait<JsonType>>, bool> = true>
void jsonSerialize(OStream &os, const JsonType &value);

template <class OStream, class IStream, typename JsonType,
          std::enable_if_t<std::is_class_v<JsonTrait<JsonType>>, bool> = true>
JsonType jsonDeserialize(IStream &, bool &);

template <class Container, typename = void> struct ContainerReserver {
  static void reserve(Container & /*c*/, SizeType /*size*/) {}
};

template <typename Container>
struct ContainerReserver<
    Container, std::void_t<decltype(std::declval<Container>().reserve(0))>> {
  static void reserve(Container &c, SizeType size) {
    c.reserve(static_cast<size_t>(size));
  }
};

template <class Container, typename = void> struct ContainerPureValueType {
  using type =
      std::remove_const_t<nstl::pure_type_t<typename Container::value_type>>;
};

template <typename Container>
struct ContainerPureValueType<
    Container, std::void_t<decltype(std::declval<Container>().reserve(0))>> {
  static void reserve(Container &c, SizeType size) {
    c.reserve(static_cast<size_t>(size));
  }
};

template <typename T> char *to_cstr(T *value) {
  return reinterpret_cast<char *>(value);
}

template <typename T> const char *to_cstr(const T *value) {
  return reinterpret_cast<const char *>(value);
}

} // namespace internal

using namespace nstl;

template <class OStream, class IStream, typename T, typename = void>
struct Serializer {
  static void serialize(OStream &os, const T &value) {
    Impl::template serialize<T>(os, value);
  }
  static T deserialize(IStream &is, bool &good) {
    return Impl::template deserialize<T>(is, good);
  }

  struct Impl {
    mc_enable_if_is_tuplelike_(TupleLike) static void serialize(
        OStream &os, const TupleLike &value) {
      Serializer<OStream, IStream, typename TupleLike::data_type>::serialize(
          os, value._data);
    }

    mc_enable_if_is_tuplelike_(TupleLike) static TupleLike
        deserialize(IStream &is, bool &good) {
      return Serializer<OStream, IStream,
                        typename TupleLike::data_type>::deserialize(is, good);
    }

    mc_enable_if_is_number_or_enum_(NumberOrEnum) static void serialize(
        OStream &os, const NumberOrEnum &value) {
      os.write(internal::to_cstr(&value), sizeof(NumberOrEnum));
    }

    mc_enable_if_is_number_or_enum_(NumberOrEnum) static NumberOrEnum
        deserialize(IStream &is, bool &good) {
      auto value = NumberOrEnum{};
      is.read(internal::to_cstr(&value), sizeof(NumberOrEnum));
      good = !is.fail();
      return value;
    }

    mc_enable_if_is_ptr_(PointerType) static void serialize(
        OStream &os, const PointerType &p) {
      // Won't serialize type that is not default_constructible because later
      // won't be able to create object of that types to deserialize
      mc_must_default_constructible(PointerType);
      char c = 1;
      if (p) {
        using NormalTypeOfPointerType =
            std::remove_const_t<std::remove_pointer_t<PointerType>>;
        os.write(&c, 1);
        Serializer<OStream, IStream, NormalTypeOfPointerType>::serialize(os,
                                                                         *p);
      } else {
        c = 0;
        os.write(&c, 1);
      }
    }

    mc_enable_if_is_ptr_(
        PointerType) static pure_type_t<PointerType> deserialize(IStream &is,
                                                                 bool &good) {
      mc_must_default_constructible(PointerType);

      using NormalTypeOfPointerType =
          std::remove_const_t<std::remove_pointer_t<PointerType>>;
      auto p = pure_type_t<PointerType>{nullptr};
      char isNotNull = 0;

      is.read(&isNotNull, 1);
      if (isNotNull) {
        p = new NormalTypeOfPointerType{};
        *p = Serializer<OStream, IStream, NormalTypeOfPointerType>::deserialize(
            is, good);
      }
      return p;
    }

    mc_enable_if_is_smartptr_(SmartPtrType) static void serialize(
        OStream &os, const SmartPtrType &p) {
      using PtrType = typename SmartPtrType::element_type *;
      Serializer<OStream, IStream, PtrType>::serialize(os, p.get());
    }

    mc_enable_if_is_smartptr_(
        SmartPtrType) static pure_type_t<SmartPtrType> deserialize(IStream &is,
                                                                   bool &good) {
      using PtrType = typename SmartPtrType::element_type *;
      auto sptr = pure_type_t<SmartPtrType>{};

      if (auto ptr =
              Serializer<OStream, IStream, PtrType>::deserialize(is, good);
          good) {
        sptr.reset(ptr);
      }
      return sptr;
    }

    mc_enable_if_is_json_(JsonType) static void serialize(
        OStream &os, const JsonType &value) {
      internal::jsonSerialize<JsonType>(os, value);
    }

    mc_enable_if_is_json_(JsonType) static JsonType
        deserialize(IStream &is, bool &good) {
      return internal::jsonDeserialize<JsonType>(is, good);
    }

    mc_enable_if_is_iterable_(Container) static void serialize(
        OStream &os, const Container &c) {
      auto numberOfElems = static_cast<SizeType>(c.size());
      auto elemSrz =
          Serializer<OStream, IStream, typename Container::value_type>{};
      Serializer<OStream, IStream, SizeType>::serialize(os, numberOfElems);

      for (const auto &elem : c) {
        elemSrz.serialize(os, elem);
      }
    }

    template <typename Container,
              std::enable_if_t<nstl::is_iterable_v<Container>, bool> = true>
    static Container deserialize(IStream &is, bool &good) {
      using ElemType = std::remove_const_t<typename Container::value_type>;
      using SizeTypeSerializer = Serializer<OStream, IStream, SizeType>;
      using ElemSerializer = Serializer<OStream, IStream, ElemType>;
      constexpr bool pushBackable = nstl::is_back_insertible_v<Container>;
      auto c = Container{};
      if (auto size = SizeTypeSerializer::deserialize(is, good);
          good && size > 0) {
        internal::ContainerReserver<Container>::reserve(c, size);
        for (SizeType i = 0; i < size; ++i) {
          if (auto elem = ElemSerializer::deserialize(is, good); good) {
            if constexpr (pushBackable) {
              c.push_back(std::move(elem));
            } else {
              c.insert(std::move(elem));
            }
          } else {
            break;
          }
        }
      }
      return c;
    }
  };
};

template <class OStream, class IStream, typename First, typename Second>
struct Serializer<OStream, IStream, std::pair<First, Second>, void> {

  template <typename T> using MySerializer = Serializer<OStream, IStream, T>;

  using DType = std::pair<First, Second>;
  using FirstPure = pure_type_t<First>;
  using SecondPure = pure_type_t<Second>;
  using NCDType = std::pair<FirstPure, SecondPure>;

  static void serialize(OStream &os, const DType &p) {
    MySerializer<First>::serialize(os, p.first);
    MySerializer<Second>::serialize(os, p.second);
  }

  static NCDType deserialize(IStream &is, bool &good) {
    NCDType value;
    if (value.first = MySerializer<FirstPure>::deserialize(is, good); good) {
      value.second = MySerializer<SecondPure>::deserialize(is, good);
    }
    return value;
  }
};

template <class OStream, class IStream, typename Tuple>
struct Serializer<OStream, IStream, Tuple,
                  std::enable_if_t<nstl::is_tuple_v<Tuple>, void>> {
  using SrType = Tuple;

  static void serialize(OStream &os, const SrType &tp) {
    nstl::tuple_for_each(tp, [&os](const auto &elem) {
      Serializer<OStream, IStream, pure_type_t<decltype(elem)>>::serialize(
          os, elem);
    });
  }

  static SrType deserialize(IStream &is, bool &good) {
    SrType tp;
    nstl::tuple_for_each(tp, [&is, &good](auto &elem) {
      elem = Serializer<OStream, IStream,
                        pure_type_t<decltype(elem)>>::deserialize(is, good);
    });
    return tp;
  }
};

template <class OStream, class IStream, typename CharT, class Trait,
          class Allocator>
struct Serializer<OStream, IStream, std::basic_string<CharT, Trait, Allocator>,
                  void> {
  using SrType = std::basic_string<CharT>;

  static void serialize(OStream &os, const SrType &value) {
    auto size = value.size();
    os.write(internal::to_cstr(&size), sizeof(SizeType));
    if (!value.empty()) {
      os.write(reinterpret_cast<const char *>(value.c_str()),
               value.size() * sizeof(CharT));
    }
  }

  static SrType deserialize(IStream &is, bool &good) {
    SrType value;
    if (auto size =
            Serializer<OStream, IStream, SizeType>::deserialize(is, good);
        good && size > 0) {
      value.resize(size);
      is.read(internal::to_cstr(&value[0]), size * sizeof(CharT));
    }
    good = !is.fail();
    return value;
  }
};

template <class OStream, class IStream, typename StringDerived>
struct Serializer<
    OStream, IStream, StringDerived,
    std::enable_if_t<std::is_base_of_v<std::string, StringDerived> &&
                         !std::is_same_v<std::string, StringDerived>,
                     void>> {
  using SrType = StringDerived;
  static void serialize(OStream &os, const SrType &value) {
    Serializer<OStream, IStream, std::string>::serialize(
        os, static_cast<const std::string &>(value));
  }

  static SrType deserialize(IStream &is, bool &good) {
    SrType value;
    static_cast<std::string &>(value) =
        Serializer<OStream, IStream, std::string>::deserialize(is, good);
    return value;
  }
};

template <class OStream, typename T> void serialize(OStream &os, const T &v) {
  Serializer<OStream, void, T>::serialize(os, v);
}

template <class IStream, typename T> T deserialize(IStream &is, bool &good) {
  return Serializer<void, IStream, T>::deserialize(is, good);
}

template<class OStream>
class SR
{
    OStream& os_;
public:
    SR(OStream& os) : os_{os}{}
    template<typename T>
    SR& operator<<(const T& value) {
        serialize<OStream, T>(os_, value);
        return *this;
    }
};

template<class IStream>
class DSR
{
    IStream& is_;
public:
    DSR(IStream& is) : is_{is}{}
    template<typename T>
    DSR& operator>>(T& value) {
        bool good = true;
        value = deserialize<IStream, T>(is_, good);
        if(!good)
        {
            throw std::runtime_error{"Could not deserialize"};
        }
        return *this;
    }
};

namespace internal {

template <class OStream, class IStream, typename JsonType,
          std::enable_if_t<std::is_class_v<JsonTrait<JsonType>>, bool>>
void jsonSerialize(OStream &os, const JsonType &v) {
  Serializer<OStream, IStream, std::string>::serialize(
      os, JsonTrait<JsonType>::marshall(v));
}

template <class OStream, class IStream, typename JsonType,
          std::enable_if_t<std::is_class_v<JsonTrait<JsonType>>, bool>>
JsonType jsonDeserialize(IStream &is, bool &good) {
  JsonType j;
  if (std::string str =
          Serializer<OStream, IStream, std::string>::deserialize(is, good);
      good) {
    j = JsonTrait<JsonType>::unmarshall(str);
  }
  return j;
}


} // namespace internal

} // namespace srz
} // namespace maf

#undef mc_enable_if_is_tuplelike_
#undef mc_enable_if_is_number_or_enum_
#undef mc_enable_if_is_smartptr_
#undef mc_enable_if_is_ptr_
#undef mc_enable_if_is_a_char_string
#undef mc_must_default_constructible
#undef mc_enable_if_is_json_
#undef mc_enable_if_is_iterable_

#endif // SERIALIZER1_H
