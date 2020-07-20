#pragma once

#include "JsonTrait.h"
#include "Tuplizable.h"
#include <maf/utils/cppextension/TupleManip.h>
#include <maf/utils/cppextension/TypeTraits.h>
#include <string.h>
#include <string>

namespace maf {
namespace srz {

using namespace nstl;
constexpr inline size_t IndentSize = 2;
inline int nextLevel(int currentLevel) {
  if (currentLevel < 0) {
    return currentLevel;
  } else {
    return currentLevel + 1;
  }
}
inline std::string getIndent(int indentLevel, bool newLine = false) {
  if (indentLevel < 0) {
    return "";
  }
  if (newLine) {
    auto str =
        std::string(static_cast<size_t>(indentLevel * IndentSize) + 1, ' ');
    str[0] = '\n';
    return str;
  } else {
    return std::string(static_cast<size_t>(indentLevel * IndentSize), ' ');
  }
}

inline std::string keyValueSeparator(int indentLevel) {
  return indentLevel < 0 ? ":" : ": ";
}
struct hlp {
  template <typename T,
            std::enable_if_t<nstl::is_number_type_v<T>, bool> = true>
  inline static std::string quote(T value) {
    return quote(std::to_string(value));
  }
  inline static std::string quote(const std::string &str) {
    return '"' + str + '"';
  }
  inline static std::string quoteAllEscapes(const std::string &s);
};

template <typename NonDeterminedType, typename = void> struct DumpHelper {
  inline static void dump(const NonDeterminedType &value, int indentLevel,
                          std::string &strOut) noexcept {
    return prv::template dump<NonDeterminedType>(value, indentLevel, strOut);
  }

  struct prv {
    template <typename TupleLike,
              std::enable_if_t<is_tuplizable_type_v<TupleLike>, bool> = true>
    inline static void dump(const TupleLike &value, int indentLevel,
                            std::string &strOut) noexcept {
      value.dump(indentLevel, strOut);
    }

    template <typename NumberType,
              std::enable_if_t<nstl::is_number_type_v<NumberType>, bool> = true>
    inline static void dump(const NumberType &value, int /*indentLevel*/,
                            std::string &strOut) noexcept {
      strOut += std::to_string(value);
    }

    template <typename EnumType,
              std::enable_if_t<std::is_enum_v<EnumType>, bool> = true>
    inline static void dump(const EnumType &value, int /*indentLevel*/,
                            std::string &strOut) noexcept {
      strOut += std::to_string(static_cast<uint32_t>(value));
    }

    template <typename PointerType,
              std::enable_if_t<std::is_pointer_v<PointerType>, bool> = true>
    inline static void dump(const PointerType &value, int indentLevel,
                            std::string &strOut) noexcept {
      using NormalTypeOfPointerType =
          std::remove_const_t<std::remove_pointer_t<PointerType>>;
      if (value) {
        DumpHelper<NormalTypeOfPointerType>::dump(*value, indentLevel, strOut);
      } else {
        DumpHelper<NormalTypeOfPointerType>::dump(NormalTypeOfPointerType{},
                                                  indentLevel, strOut);
      }
    }

    template <typename SmartPtrType,
              std::enable_if_t<nstl::is_smart_ptr_v<SmartPtrType>, bool> = true>
    inline static void dump(const SmartPtrType &value, int indentLevel,
                            std::string &strOut) noexcept {
      using PtrType = typename SmartPtrType::element_type *;
      DumpHelper<PtrType>::dump(value.get(), indentLevel, strOut);
    }

    template <
        typename JsonType,
        std::enable_if_t<is_maf_compatible_json<JsonType>::value, bool> = true>
    inline static void dump(const JsonType &value, int /*indentLevel*/,
                            std::string &strOut) noexcept {
      strOut += JsonTrait<JsonType>::marshall(value);
    }
  };
};

template <class JsonClass> struct DumpHelper<JsonTrait<JsonClass>, void> {
  inline static void dump(const bool &value, int /*indentLevel*/,
                          std::string &strOut) noexcept {
    strOut += value ? "true" : "false";
  }
};

template <> struct DumpHelper<bool, void> {
  inline static void dump(const bool &value, int /*indentLevel*/,
                          std::string &strOut) noexcept {
    strOut += value ? "true" : "false";
  }
};

template <typename T1, typename T2> struct DumpHelper<std::pair<T1, T2>, void> {
  using DType = std::pair<T1, T2>;

  inline static void dump(const DType &p, int indentLevel,
                          std::string &strOut) noexcept {
    DumpHelper<pure_type_t<decltype(p.first)>>::dump(p.first, indentLevel,
                                                     strOut);
    strOut += keyValueSeparator(indentLevel);
    DumpHelper<pure_type_t<decltype(p.second)>>::dump(p.second, indentLevel,
                                                      strOut);
  }
};

template <typename Tuple>
struct DumpHelper<Tuple, std::enable_if_t<nstl::is_tuple_v<Tuple>, void>> {
  inline static void dump(const Tuple &tp, int indentLevel,
                          std::string &strOut) noexcept {
    constexpr bool newLine = true;
    strOut += "[";
    nstl::tuple_for_each(tp,
                         [indentLevel, &strOut, &newLine](const auto &elem) {
                           strOut += getIndent(nextLevel(indentLevel), newLine);
                           DumpHelper<pure_type_t<decltype(elem)>>::dump(
                               elem, nextLevel(indentLevel), strOut);
                           strOut += ",";
                         });

    strOut.resize(strOut.size() - 1); // remove the last ',' character
    strOut += getIndent(indentLevel, newLine) + "]";
  }
};

template <> struct DumpHelper<std::string, void> {
  inline static void dump(const std::string &value, int /*indentLevel*/,
                          std::string &strOut) noexcept {
    // must be taken care for case of wstring
    strOut += hlp::quoteAllEscapes(value);
  }
};

template <class StringType>
struct DumpHelper<
    StringType,
    std::enable_if_t<std::is_constructible_v<std::string, StringType> ||
                         std::is_base_of_v<std::string, StringType>,
                     void>> {
  inline static void dump(const std::string &value, int /*indentLevel*/,
                          std::string &strOut) noexcept {
    // must be taken care for case of wstring
    strOut += hlp::quoteAllEscapes(value);
  }
};

// template<typename StringDerived>
// struct DumpHelper<StringDerived,
//                  std::enable_if_t<std::is_base_of_v<std::string,
//                  StringDerived>, void>>
//{

//    inline static void dump(const StringDerived& value, int /*indentLevel*/,
//    std::string& strOut) noexcept {
//        //must be taken care for case of wstring
//        strOut += hlp::quoteAllEscapes(static_cast<const
//        std::string&>(value));
//    }
//};

// template<>
// struct DumpHelper<const char*, void>
//{
//    inline static void dump(const char* value, int /*indentLevel*/,
//    std::string& strOut) noexcept {
//        //must be taken care for case of wstring
//        strOut += hlp::quoteAllEscapes(std::string{value});
//    }
//};

template <> struct DumpHelper<std::wstring, void> {
  inline static void dump(const std::wstring & /*wvalue*/, int /*indentLevel*/,
                          std::string &strOut) noexcept {
    //        std::string value(wvalue.size() * sizeof(wchar_t), '\0');
    //        memcpy(value.data(), wvalue.data(), value.size());
    //        strOut += hlp::quote(value);
    strOut += "wstring hasn't been supported now";
  }
};

template <typename Iterable, typename = void> struct Packager {
  inline static void openBox(int /*indentLevel*/, std::string &strOut) {
    strOut += "[";
  }
  inline static void closeBox(int indentLevel, std::string &strOut) {
    strOut += getIndent(indentLevel, true) + "]";
  }
};

template <typename AssociateContainer>
struct Packager<AssociateContainer,
                nstl::to_void<typename AssociateContainer::key_type,
                              typename AssociateContainer::mapped_type>> {
  inline static void openBox(int /*indentLevel*/, std::string &strOut) {
    strOut += "{";
  }
  inline static void closeBox(int indentLevel, std::string &strOut) {
    strOut += getIndent(indentLevel, true) + "}";
  }
};

template <class Iterable>
struct DumpHelper<Iterable,
                  std::enable_if_t<nstl::is_iterable_v<Iterable>, void>> {
  inline static void dump(const Iterable &seq, int indentLevel,
                          std::string &strOut) noexcept {
    constexpr bool NEWLINE = true;
    auto elemPreSeparator = getIndent(nextLevel(indentLevel), NEWLINE);
    Packager<Iterable>::openBox(indentLevel, strOut);
    for (const auto &elem : seq) {
      strOut += elemPreSeparator;
      DumpHelper<typename Iterable::value_type>::dump(
          elem, nextLevel(indentLevel), strOut);
      strOut += ",";
    }
    if (!seq.empty()) {
      strOut.resize(strOut.size() - 1); // remove the last ',' character
    }
    Packager<Iterable>::closeBox(indentLevel, strOut);
  }
};

template <typename T>
void dump(const T &val, int indentLevel, std::string &strOut) {
  DumpHelper<std::decay_t<decltype(val)>>::dump(val, indentLevel, strOut);
}

template <typename T> std::string dump(const T &val, int indenLevel = 0) {
  std::string strOut;
  dump(val, indenLevel, strOut);
  return strOut;
}

std::string hlp::quoteAllEscapes(const std::string &value) {
  std::string out = "\"";
  for (auto it = std::begin(value); it != std::end(value); ++it) {
    auto ch = *it;
    if (ch == '\\') {
      out += "\\\\";
    } else if (ch == '"') {
      out += "\\\"";
    } else if (ch == '\b') {
      out += "\\b";
    } else if (ch == '\f') {
      out += "\\f";
    } else if (ch == '\n') {
      out += "\\n";
    } else if (ch == '\r') {
      out += "\\r";
    } else if (ch == '\t') {
      out += "\\t";
    } else if (static_cast<uint8_t>(ch) <= 0x1f) {
      char buf[8];
      snprintf(buf, sizeof buf, "\\u%04x", ch);
      out += buf;
    } else if (static_cast<uint8_t>(ch) == 0xe2 &&
               static_cast<uint8_t>(*(it + 1)) == 0x80 &&
               static_cast<uint8_t>(*(it + 2)) == 0xa8) {
      out += "\\u2028";
      it += 2;
    } else if (static_cast<uint8_t>(ch) == 0xe2 &&
               static_cast<uint8_t>(*(it + 1)) == 0x80 &&
               static_cast<uint8_t>(*(it + 2)) == 0xa9) {
      out += "\\u2029";
      it += 2;
    } else {
      out += ch;
    }
  }
  out += '"';
  return out;
}

} // namespace srz
} // namespace maf
