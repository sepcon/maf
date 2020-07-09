#pragma once

#include "cppextension/Loop.mc.h"
#include <ostream>

/// Allow you to declare enum and stringify its values as the values' name using
/// std::ostream.
/// e.g: MC_MAF_STRINGIFYABLE_ENUM(EnumColor, char,
///                                 Red,
///                                 Green,
///                                 Blue,
///                                 ...)
///
/// after that you can write the enum value to std::cout like:
/// ---
/// void logEnum(EnumColor color) {
///   std::cout << "The color is  " << color << std::endl;
/// }
///
/// --
/// main() {
///   logEnum(EnumColor::Red);
///   logEnum(EnumColor::Green);
///   logEnum(EnumColor::Blue);
/// }
///
/// ---
/// OUTPUT:
///  The color is Red
///  The color is Green
///  The color is Blue
/// ---
/// WARN: you are not allow to use this macro to declare enum with custom
/// values like:
/// MC_MAF_STRINGIFYABLE_ENUM(EnumColor, char,
///                             Red = 0,
///                             Green = 127,
///                             Blue = 255,
///                             ...)
/// it generates:
/// const char* EnumColorStrArray[] = { "Red", "Green", "Blue", ...,
/// "EnumColorInvalid" }; Then try to convert to string EnumColor::Green by get
/// the value of EnumColorStrArray[static_cast<char>(EnumColor::Green)] ==
/// EnumColorStrArray[127]
/// --> undefined behaviour.

#define MC_MAF_STRINGIFY(arg) #arg,

#define MC_MAF_STRINGIFYABLE_ENUM(TheEnum, base_type, ...)                     \
  enum class TheEnum : base_type { __VA_ARGS__, TheEnum##InvalidPlaceHolder }; \
                                                                               \
  constexpr const char *TheEnum##StrArr[] = {                                  \
      mc_maf_for_each(MC_MAF_STRINGIFY, __VA_ARGS__) #TheEnum "Invalid"};      \
  inline std::ostream &operator<<(std::ostream &os, TheEnum en) {              \
    os << TheEnum##StrArr[static_cast<unsigned long long>(en)];                \
    return os;                                                                 \
  }                                                                            \
  static_assert(                                                               \
      static_cast<base_type>(TheEnum::TheEnum##InvalidPlaceHolder) ==          \
          sizeof(TheEnum##StrArr) / sizeof(const char *) - 1,                  \
      "Max enum value of " #TheEnum " must equal to number of its values, "    \
      "Because StringifyableEnum doesnot "                                     \
      "support Enum with custom values");
