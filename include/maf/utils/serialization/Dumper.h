#pragma once

#include <maf/utils/cppextension/TupleManip.h>
#include <string.h>

#include <sstream>
#include <string>
#include <string_view>

#include "JsonTrait.h"
#include "Tuplizable.h"

namespace maf {
namespace srz {

using namespace nstl;

template <class OStream, typename T>
void _dump(OStream &ds, const T &val, int indentLevel = 0);
template <class OStream, typename T>
void dump(OStream &ds, const T &val, int indentLevel = 0);
template <typename T>
std::string toString(const T &val, int indenLevel = 0);

constexpr inline size_t IndentSize = 2;

inline int nextLevel(int currentLevel) {
  if (currentLevel < 0) {
    return currentLevel;
  } else {
    return currentLevel + 1;
  }
}

template <class _OStream>
inline _OStream &writeIndent(_OStream &os, int indentLevel,
                             bool newLine = false) {
  if (indentLevel >= 0) {
    if (newLine) {
      os << '\n';
    }
    for (size_t i = 0; i < indentLevel * IndentSize; ++i) {
      os << ' ';
    }
  }
  return os;
}

template <class _OStream>
inline _OStream &keyValueSeparator(_OStream &os, int indentLevel) {
  os << (indentLevel < 0 ? ":" : ": ");
  return os;
}

namespace details {

template <class OStream, typename T, typename = void>
struct DumperSFINAE {
  static void write(OStream &ds, const T &, int) noexcept {
    ds << "unsupported type " << typeid(T).name();
  }
};

template <typename Pointer>
struct PointerDumpable {
  static constexpr bool value =
      std::is_pointer_v<Pointer> &&
      !std::is_abstract_v<std::remove_const_t<std::remove_pointer_t<Pointer>>>;
};
template <class OStream, typename PointerType>
struct DumperSFINAE<
    OStream, PointerType,
    std::enable_if_t<PointerDumpable<PointerType>::value, void>> {
  static void write(OStream &ds, const PointerType &value,
                    int indentLevel) noexcept {
    using NormalTypeOfPointerType =
        std::remove_const_t<std::remove_pointer_t<PointerType>>;
    using namespace maf::srz;

    if (value) {
      dump(ds, *value, indentLevel);
    } else {
      dump(ds, NormalTypeOfPointerType(), indentLevel);
    }
  }
};

template <class OStream, typename SmartPtrType>
struct DumperSFINAE<
    OStream, SmartPtrType,
    std::enable_if_t<nstl::is_smart_ptr_v<SmartPtrType>, void>> {
  static void write(OStream &ds, const SmartPtrType &value,
                    int indentLevel) noexcept {
    using namespace maf::srz;
    dump(ds, value.get(), indentLevel);
  }
};

template <class OStream, class JsonType>
struct DumperSFINAE<
    OStream, JsonType,
    std::enable_if_t<is_maf_compatible_json<JsonType>::value, void>> {
  static void write(OStream &ds, const JsonType &value, int /*indentLevel*/
                    ) noexcept {
    ds << JsonTrait<JsonType>::marshall(value);
  }
};

template <class OStream, class EnumType>
struct DumperSFINAE<OStream, EnumType,
                    std::enable_if_t<std::is_enum_v<EnumType>, void>> {
  static void write(OStream &ds, const EnumType &value, int /*indentLevel*/
                    ) noexcept {
    ds << static_cast<uint32_t>(value);
  }
};

template <class OStream, class NumberType>
struct DumperSFINAE<
    OStream, NumberType,
    std::enable_if_t<nstl::is_number_type_v<NumberType>, void>> {
  static void write(OStream &ds, const NumberType &value, int /*indentLevel*/) {
    ds << value;
  }
};

template <class OStream, class Tuplizable>
struct DumperSFINAE<
    OStream, Tuplizable,
    std::enable_if_t<has_cas_tuple_method<Tuplizable>::value, void>> {
  static void write(OStream &ds, const Tuplizable &value,
                    int indentLevel) noexcept {
    dump(ds, value.cas_tuple(), indentLevel);
  }
};

template <class OStream, typename Iterable, typename = void>
struct Packager {
  inline static void openBox(OStream &ds, int /*indentLevel*/) { ds << "["; }
  inline static void closeBox(OStream &ds, int indentLevel) {
    writeIndent(ds, indentLevel, true) << "]";
  }
};

template <class OStream, typename AssociateContainer>
struct Packager<OStream, AssociateContainer,
                std::void_t<typename AssociateContainer::key_type,
                            typename AssociateContainer::mapped_type>> {
  inline static void openBox(OStream &ds, int /*indentLevel*/) { ds << "{"; }
  inline static void closeBox(OStream &ds, int indentLevel) {
    writeIndent(ds, indentLevel, true) << "}";
  }
};

template <class OStream, class Iterable>
struct DumperSFINAE<OStream, Iterable,
                    std::enable_if_t<nstl::is_iterable_v<Iterable>, void>> {
  static void write(OStream &ds, const Iterable &seq, int indentLevel) {
    using namespace maf::srz;
    constexpr bool NEWLINE = true;
    auto nextIndentLevel = nextLevel(indentLevel);
    Packager<OStream, Iterable>::openBox(ds, indentLevel);
    auto beg = std::begin(const_cast<Iterable &>(seq));
    auto ed = std::end(const_cast<Iterable &>(seq));
    if (beg != ed) {
      writeIndent(ds, nextIndentLevel, NEWLINE);
      dump(ds, *beg, nextLevel(indentLevel));

      while (++beg != ed) {
        ds << ",";
        writeIndent(ds, nextIndentLevel, NEWLINE);
        dump(ds, *beg, nextLevel(indentLevel));
      }
    }
    Packager<OStream, Iterable>::closeBox(ds, indentLevel);
  }
};
}  // namespace details

template <class OStream>
void _dumpString(OStream &os, const std::string_view &s, int /*indentLevel*/) {
  os << s;
  //  os << "\"";
  //  for (auto it = std::begin(s); it != std::end(s); ++it) {
  //    auto ch = *it;
  //    if (ch == '\\') {
  //      os << "\\\\";
  //    } else if (ch == '"') {
  //      os << "\\\"";
  //    } else if (ch == '\b') {
  //      os << "\\b";
  //    } else if (ch == '\f') {
  //      os << "\\f";
  //    } else if (ch == '\n') {
  //      os << "\\n";
  //    } else if (ch == '\r') {
  //      os << "\\r";
  //    } else if (ch == '\t') {
  //      os << "\\t";
  //    } else if (static_cast<uint8_t>(ch) <= 0x1f) {
  //      char buf[8];
  //      snprintf(buf, sizeof buf, "\\u%04x", ch);
  //      os << buf;
  //    } else if (static_cast<uint8_t>(ch) == 0xe2 &&
  //               static_cast<uint8_t>(*(it + 1)) == 0x80 &&
  //               static_cast<uint8_t>(*(it + 2)) == 0xa8) {
  //      os << "\\u2028";
  //      it += 2;
  //    } else if (static_cast<uint8_t>(ch) == 0xe2 &&
  //               static_cast<uint8_t>(*(it + 1)) == 0x80 &&
  //               static_cast<uint8_t>(*(it + 2)) == 0xa9) {
  //      os << "\\u2029";
  //      it += 2;
  //    } else {
  //      os << ch;
  //    }
  //  }
  //  os << '"';
}

template <class OStream, typename T1, typename T2>
void _dump(OStream &ds, const std::pair<T1, T2> &val, int indentLevel) {
  dump(ds, val.first, indentLevel);
  keyValueSeparator(ds, indentLevel);
  dump(ds, val.second, indentLevel);
}

template <class OStream>
void _dump(OStream &ds, bool val, int /*indentLevel*/) {
  ds << (val ? "true" : "false");
}

template <class OStream, typename T>
void _dump(OStream &ds, const T &val, int indentLevel) {
  details::DumperSFINAE<OStream, T>::write(ds, val, indentLevel);
}

template <class OStream>
void _dump(OStream &ds, const std::string_view &val, int indentLevel) {
  _dumpString(ds, val, indentLevel);
}

template <class OStream, size_t size>
void _dump(OStream &ds, const char val[size], int indentLevel) {
  _dumpString(ds, val, indentLevel);
}

template <class OStream>
void _dump(OStream &ds, const std::string &val, int indentLevel) {
  _dumpString(ds, val, indentLevel);
}
template <class OStream>
void _dump(OStream &ds, const std::wstring & /*val*/, int indentLevel) {
  _dumpString(ds, "wstring hasn't been supported now", indentLevel);
}

template <class OStream>
void _dump(OStream &ds, const char *val, int indentLevel) {
  _dumpString(ds, val, indentLevel);
}

template <class OStream>
void _dump(OStream &ds, char *val, int indentLevel) {
  _dumpString(ds, val, indentLevel);
}

template <class OStream, typename... _Args>
void _dump(OStream &ds, const std::tuple<_Args...> &val, int indentLevel) {
  using Tuple = std::tuple<_Args...>;
  ds << "[";
  size_t i = 0;
  auto nextIndent = nextLevel(indentLevel);
  nstl::tuple_for_each(val, [&](const auto &elem) {
    writeIndent(ds, nextIndent, true);
    dump(ds, elem, nextIndent);
    if (++i < std::tuple_size_v<Tuple>) {
      ds << ",";
    }
  });
  writeIndent(ds, indentLevel, true) << "]";
}

template <class OStream, typename T>
void dump(OStream &ds, const T &val, int indentLevel) {
  using namespace maf::srz;
  _dump(ds, val, indentLevel);
}

template <typename T>
std::string toString(const T &val, int indenLevel) {
  std::ostringstream oss;
  dump(oss, val, indenLevel);
  return oss.str();
}

template <class OStream>
struct _DumpStream {
  _DumpStream(OStream &os, int indent = 0) : os_{os}, indent_{indent} {}
  OStream &os_;
  int indent_ = 0;
};

struct Indent {
  int value_ = 0;
  operator int() const { return value_; }
};

inline auto indent(int i) { return Indent{i}; }

template <class T>
struct __NoDump {
  const T &ref;
  __NoDump(const T &v) : ref(v) {}
  operator const T &() const { return ref; }
};

template <class T>
inline auto nodump(const T &v) {
  return __NoDump<T>(v);
}

template <class OStream>
inline void newline(_DumpStream<OStream> &ds) {
  ds.os_ << '\n';
}

template <class OStream>
_DumpStream<OStream> &operator<<(_DumpStream<OStream> &ds, Indent i) {
  ds.indent_ = i;
  return ds;
}

template <class OStream, class T>
_DumpStream<OStream> &operator<<(_DumpStream<OStream> &ds, __NoDump<T> v) {
  ds.os_ << v.ref;
  return ds;
}

template <class OStream>
_DumpStream<OStream> &operator<<(_DumpStream<OStream> &ds,
                                 void (*manip)(_DumpStream<OStream> &)) {
  manip(ds);
  return ds;
}

template <class OStream, class T>
_DumpStream<OStream> &operator<<(_DumpStream<OStream> &ds, const T &v) {
  dump(ds.os_, v, ds.indent_);
  return ds;
}

template <class OStream>
auto dumpstream(OStream &os, int indent = 0) {
  return _DumpStream<OStream>(os, indent);
}

}  // namespace srz
}  // namespace maf

#define MC_MAF_DEFINE_DUMP_FUNCTION(CustomType, variableName) \
  template <class OStream>                                    \
  void _dump(OStream &os, const CustomType &variableName, int indentLevel = 0)
