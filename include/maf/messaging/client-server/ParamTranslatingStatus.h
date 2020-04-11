#ifndef MESSAGETRAITBASE_H
#define MESSAGETRAITBASE_H

#include <maf/utils/StringifyableEnum.h>

namespace maf {
namespace messaging {

// clang-format off
MC_MAF_STRINGIFYABLE_ENUM(TranslationStatus, char,
                          Success,
                          NoSource,
                          DestSrcMismatch
                          );
// clang-format on

} // namespace messaging
} // namespace maf

#endif // MESSAGETRAITBASE_H
