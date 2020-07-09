#pragma once

#include <maf/utils/StringifyableEnum.h>

namespace maf {
namespace messaging {

// clang-format off
MC_MAF_STRINGIFYABLE_ENUM(TranslationStatus, char,
                          Success,
                          NoSource,
                          SourceCorrupted,
                          DestSrcMismatch
                          );
// clang-format on

} // namespace messaging
} // namespace maf
