#pragma once

#include <maf/utils/cppextension/TupleManip.h>
#include <maf/utils/cppextension/TypeTraits.h>

#include <string>

#include "Tuplizable.h"

/// Serialization

#define mc_enable_if_is_number_or_enum_(NumberType)                     \
  template <typename NumberType,                                        \
            std::enable_if_t<nstl::is_number_type<NumberType>::value || \
                                 std::is_enum_v<NumberType>,            \
                             bool> = true>
#define mc_enable_if_is_smartptr_(SmartPtrType) \
  template <typename SmartPtrType,              \
            std::enable_if_t<nstl::is_smart_ptr_v<SmartPtrType>, bool> = true>
#define mc_enable_if_is_ptr_(PointerType) \
  template <typename PointerType,         \
            std::enable_if_t<std::is_pointer_v<PointerType>, bool> = true>
#define mc_enable_if_is_a_char_string(CharString)                          \
  template <typename CharString,                                           \
            std::enable_if_t<std::is_base_of_v<std::string, CharString> && \
                                 !std::is_same_v<std::string, CharString>, \
                             bool> = true>
#define mc_must_default_constructible(PointerType)                         \
  static_assert(                                                           \
      std::is_default_constructible_v<std::remove_pointer_t<PointerType>>, \
      "");

#define mc_enable_if_is_iterable_(Container) \
  template <typename Container,              \
            std::enable_if_t<nstl::is_iterable_v<Container>, bool> = true>

namespace maf {
namespace srz {

using namespace nstl;

using SizeType = uint32_t;
inline constexpr SizeType SIZETYPE_WIDE = sizeof(SizeType);

template <class OStream, typename T>
void serialize(OStream &, const T &);

template <class IStream, typename T>
bool deserialize(IStream &is, T &value);

template <typename T>
SizeType serializedSize(const T &value);

namespace internal {

template <class Container, typename = void>
struct ContainerReserver {
  static void reserve(Container & /*c*/, SizeType /*size*/) {}
};

template <typename Container>
struct ContainerReserver<
    Container, std::void_t<decltype(std::declval<Container>().reserve(0))>> {
  static void reserve(Container &c, SizeType size) {
    c.reserve(static_cast<size_t>(size));
  }
};

template <typename T>
char *to_cstr(T *value) {
  return reinterpret_cast<char *>(value);
}

template <typename T>
const char *to_cstr(const T *value) {
  return reinterpret_cast<const char *>(value);
}

template <class StreamType, typename = void>
struct StreamHelper {
  static void prepareNextWrite(StreamType &, SizeType) {}
};

template <class T>
struct DeserializableType {
  using Type = std::decay_t<T>;
};

template <class First, class Second>
struct DeserializableType<std::pair<First, Second>> {
  using Type = std::pair<std::decay_t<First>, std::decay_t<Second>>;
};

template <class... Args>
struct DeserializableType<std::tuple<Args...>> {
  using Type = std::tuple<std::decay_t<Args>...>;
};

}  // namespace internal

template <class OStream, class IStream, typename T, typename = void>
struct Serializer {
  template <typename TT>
  using SerializerT = Serializer<OStream, IStream, TT>;

  void serialize(OStream &os, const T &value) {
    Impl::template serialize<T>(os, value);
  }
  bool deserialize(IStream &is, T &val) {
    return Impl::template deserialize<T>(is, val);
  }

  SizeType serializedSize(const T &value) noexcept {
    return Impl::template serializedSize<T>(value);
  }

  struct Impl {
    template <
        class Tuplizable,
        std::enable_if_t<has_cas_tuple_method<Tuplizable>::value, bool> = true>
    inline static SizeType serializedSize(const Tuplizable &value) noexcept {
      return maf::srz::serializedSize(value.cas_tuple());
    }

    template <
        class Tuplizable,
        std::enable_if_t<has_cas_tuple_method<Tuplizable>::value, bool> = true>
    static void serialize(OStream &os, const Tuplizable &value) {
      maf::srz::serialize(os, value.cas_tuple());
    }

    template <
        class Tuplizable,
        std::enable_if_t<has_as_tuple_method<Tuplizable>::value, bool> = true>
    static bool deserialize(IStream &is, Tuplizable &tpl) {
      auto tp = tpl.as_tuple();
      return maf::srz::deserialize(is, tp);
    }

    template <class Struct>
    using __is_pure_struct_t =
        std::enable_if_t<std::is_aggregate_v<Struct> &&
                             !has_cas_tuple_method<Struct>::value &&
                             !has_as_tuple_method<Struct>::value,
                         bool>;

    template <class Struct, __is_pure_struct_t<Struct> = true>
    inline static SizeType serializedSize(const Struct &struct_) noexcept {
      auto tp = tuple_view(struct_);
      return maf::srz::serializedSize(tp);
    }

    template <class Struct, __is_pure_struct_t<Struct> = true>
    static void serialize(OStream &os, const Struct &struct_) {
      maf::srz::serialize(os, tuple_view(struct_));
    }

    template <class Struct, __is_pure_struct_t<Struct> = true>
    static bool deserialize(IStream &is, Struct &struct_) {
      auto tp = tuple_view(struct_);
      return maf::srz::deserialize(is, tp);
    }

    mc_enable_if_is_number_or_enum_(NumberOrEnum) static SizeType
        serializedSize(const NumberOrEnum & /*value*/) noexcept {
      return sizeof(NumberOrEnum);
    }

    mc_enable_if_is_number_or_enum_(NumberOrEnum) static void serialize(
        OStream &os, const NumberOrEnum &value) {
      os.write(internal::to_cstr(&value), sizeof(NumberOrEnum));
    }

    mc_enable_if_is_number_or_enum_(NumberOrEnum) static bool deserialize(
        IStream &is, NumberOrEnum &value) {
      is.read(internal::to_cstr(&value), sizeof(NumberOrEnum));
      return !is.fail();
    }

    mc_enable_if_is_ptr_(PointerType) inline static SizeType
        serializedSize(PointerType value) noexcept {
      SizeType size = 1;
      if (value) {
        size += maf::srz::serializedSize(*value);
      }
      return size;
    }

    mc_enable_if_is_ptr_(PointerType) static void serialize(
        OStream &os, const PointerType &p) {
      // Won't serialize type that is not default_constructible because later
      // won't be able to create object of that types to deserialize
      mc_must_default_constructible(PointerType);
      char c = 1;
      if (p) {
        os.write(&c, 1);
        maf::srz::serialize(os, *p);
      } else {
        c = 0;
        os.write(&c, 1);
      }
    }

    mc_enable_if_is_ptr_(PointerType) static bool deserialize(
        IStream &is, pure_type_t<PointerType> &p) {
      mc_must_default_constructible(PointerType);

      using NormalTypeOfPointerType =
          std::remove_const_t<std::remove_pointer_t<PointerType>>;
      auto success = true;
      p = nullptr;
      uint8_t isNotNull = 0;

      is.read(internal::to_cstr(&isNotNull), 1);
      if (isNotNull) {
        p = new NormalTypeOfPointerType{};
        success = maf::srz::deserialize(is, *p);
        if (!success) {
          delete p;
          p = nullptr;
        }
      }

      return success;
    }

    mc_enable_if_is_smartptr_(SmartPtrType) static SizeType
        serializedSize(const SmartPtrType &value) noexcept {
      return maf::srz::serializedSize(value.get());
    }

    mc_enable_if_is_smartptr_(SmartPtrType) static void serialize(
        OStream &os, const SmartPtrType &p) {
      maf::srz::serialize(os, p.get());
    }

    mc_enable_if_is_smartptr_(SmartPtrType) static bool deserialize(
        IStream &is, pure_type_t<SmartPtrType> &sptr) {
      using PtrType = typename SmartPtrType::element_type *;
      auto success = false;
      PtrType ptr = nullptr;
      if (success = maf::srz::deserialize(is, ptr); success) {
        sptr.reset(ptr);
      }
      return success;
    }

    mc_enable_if_is_iterable_(Container) inline static SizeType
        serializedSize(const Container &c) noexcept {
      SizeType contentSize = 0;
      for (const auto &e : c) {
        contentSize += maf::srz::serializedSize(e);
      }
      return SIZETYPE_WIDE + contentSize;
    }

    mc_enable_if_is_iterable_(Container) static void serialize(
        OStream &os, const Container &c) {
      auto numberOfElems = static_cast<SizeType>(c.size());
      maf::srz::serialize(os, numberOfElems);
      for (const auto &elem : c) {
        maf::srz::serialize(os, elem);
      }
    }

    template <typename Container,
              std::enable_if_t<nstl::is_iterable_v<Container>, bool> = true>
    static bool deserialize(IStream &is, Container &c) {
      using ElemType = typename Container::value_type;
      using DSBElemType = typename internal::DeserializableType<ElemType>::Type;
      auto success = false;
      SizeType size = 0;

      if (success |= maf::srz::deserialize(is, size); success && size > 0) {
        internal::ContainerReserver<Container>::reserve(c, size);
        for (SizeType i = 0; i < size; ++i) {
          DSBElemType elem;
          if (success |= maf::srz::deserialize(is, elem); success) {
            if constexpr (nstl::is_back_insertible_v<Container>) {
              c.push_back(std::move(elem));
            } else {
              c.insert(std::move(elem));
            }
          } else {
            break;
          }
        }
      }
      return success;
    }
  };
};

template <class OStream, class IStream, typename First, typename Second>
struct Serializer<OStream, IStream, std::pair<First, Second>, void> {
  template <typename T>
  using SerializerT = Serializer<OStream, IStream, T>;

  using DType = std::pair<First, Second>;
  using FirstPure = pure_type_t<First>;
  using SecondPure = pure_type_t<Second>;
  using NCDType = std::pair<FirstPure, SecondPure>;

  SizeType serializedSize(const DType &p) noexcept {
    return maf::srz::serializedSize(p.first) +
           maf::srz::serializedSize(p.second);
  }

  void serialize(OStream &os, const DType &p) {
    maf::srz::serialize(os, p.first);
    maf::srz::serialize(os, p.second);
  }

  bool deserialize(IStream &is, NCDType &value) {
    bool success = false;
    if (success |= maf::srz::deserialize(is, value.first); success) {
      success |= maf::srz::deserialize(is, value.second);
    }
    return success;
  }
};

template <class OStream, class IStream, typename Tuple>
struct Serializer<OStream, IStream, Tuple,
                  std::enable_if_t<nstl::is_tuple_v<Tuple>, void>> {
  using SrType = Tuple;

  SizeType serializedSize(const Tuple &tp) noexcept {
    SizeType contentSize = 0;
    nstl::tuple_for_each(tp, [&contentSize](const auto &elem) {
      contentSize += maf::srz::serializedSize(elem);
    });
    return contentSize;
  }

  void serialize(OStream &os, const SrType &tp) {
    nstl::tuple_for_each(
        tp, [&os](const auto &elem) { maf::srz::serialize(os, elem); });
  }

  bool deserialize(IStream &is, SrType &tp) {
    bool success = false;
    nstl::tuple_for_each(tp, [&is, &success](auto &elem) {
      success |= maf::srz::deserialize(is, elem);
    });
    return success;
  }
};

template <class OStream, class IStream, typename CharT, class Trait,
          class Allocator>
struct Serializer<OStream, IStream, std::basic_string<CharT, Trait, Allocator>,
                  void> {
  template <typename T>
  using SerializerT = Serializer<OStream, IStream, T>;
  using SrType = std::basic_string<CharT>;

  SizeType serializedSize(const SrType &value) noexcept {
    return SIZETYPE_WIDE + static_cast<SizeType>(value.size() * sizeof(CharT));
  }

  void serialize(OStream &os, const SrType &value) {
    auto size = value.size();
    os.write(internal::to_cstr(&size), sizeof(SizeType));
    if (!value.empty()) {
      os.write(reinterpret_cast<const char *>(value.c_str()),
               value.size() * sizeof(CharT));
    }
  }

  bool deserialize(IStream &is, SrType &value) {
    SizeType size = 0;
    if (maf::srz::deserialize(is, size)) {
      if (size > 0) {
        value.resize(size);
        is.read(internal::to_cstr(&value[0]), size * sizeof(CharT));
      }
    }
    return !is.fail();
  }
};

template <class OStream, class IStream, typename StringDerived>
struct Serializer<
    OStream, IStream, StringDerived,
    std::enable_if_t<std::is_base_of_v<std::string, StringDerived> &&
                         !std::is_same_v<std::string, StringDerived>,
                     void>> {
  template <typename T>
  using SerializerT = Serializer<OStream, IStream, T>;
  using SrType = StringDerived;

  SizeType serializedSize(const SrType &value) noexcept {
    return maf::srz::serializedSize(static_cast<const std::string &>(value));
  }

  void serialize(OStream &os, const SrType &value) {
    maf::srz::serialize(os, static_cast<const std::string &>(value));
  }

  bool deserialize(IStream &is, SrType &value) {
    return maf::srz::deserialize(is, static_cast<std::string &>(value));
  }
};

template <typename T>
SizeType _serializedSize(const T &value) {
  return Serializer<std::nullptr_t, std::nullptr_t, T>{}.serializedSize(value);
}

template <class OStream, typename T>
void _serialize(OStream &os, const T &v) {
  using namespace internal;
  auto sr = Serializer<OStream, std::nullptr_t, T>{};
  sr.serialize(os, v);
}

template <class IStream, typename T>
bool _deserialize(IStream &is, T &value) {
  return Serializer<std::nullptr_t, IStream, T>{}.deserialize(is, value);
}

template <class OStream, typename T>
void serialize(OStream &os, const T &v) {
  using namespace maf::srz;
  _serialize(os, v);
}

template <class IStream, typename T>
bool deserialize(IStream &is, T &value) {
  using namespace maf::srz;
  return _deserialize(is, value);
}

template <typename T>
SizeType serializedSize(const T &value) {
  using namespace maf::srz;
  return _serializedSize(value);
}

template <class OStream, typename... Ts>
void serializeBatch(OStream &os, const Ts &...ts) {
  serialize(os, std::tie(ts...));
}

template <class IStream, typename... Ts>
bool deserializeBatch(IStream &is, Ts &...ts) {
  return deserialize(is, std::tie(ts...));
}

template <class OStream>
class SR {
  OStream &os_;

 public:
  SR(OStream &os) : os_{os} {}
  template <typename T>
  SR &operator<<(const T &value) {
    serialize(os_, value);
    return *this;
  }

  template <typename... Ts>
  void serializeBatch(const Ts &...ts) {
    maf::srz::serializeBatch(os_, ts...);
  }
};

template <class IStream>
class DSR {
  IStream &is_;

 public:
  DSR(IStream &is) : is_{is} {}
  template <typename T>
  DSR &operator>>(T &value) {
    if (!deserialize(is_, value)) {
      throw std::runtime_error{"Could not deserialize"};
    }
    return *this;
  }

  template <typename... Ts>
  DSR &deserializeBatch(Ts &...ts) {
    if (!maf::srz::deserializeBatch(is_, ts...)) {
      throw std::runtime_error{"Could not deserialize"};
    }
    return *this;
  }
};

#define MC_MAF_DEFINE_SERIALIZE_FUNCTION(CustomType, variableName) \
  template <class OStream>                                         \
  void _serialize(OStream &os, const CustomType &variableName)

#define MC_MAF_DEFINE_DESERIALIZE_FUNCTION(CustomType, variableName) \
  template <class IStream>                                           \
  bool _deserialize(IStream &is, CustomType &variableName)

}  // namespace srz
}  // namespace maf

#undef mc_enable_if_is_number_or_enum_
#undef mc_enable_if_is_smartptr_
#undef mc_enable_if_is_ptr_
#undef mc_enable_if_is_a_char_string
#undef mc_must_default_constructible
#undef mc_enable_if_is_iterable_
