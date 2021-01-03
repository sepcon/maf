#pragma once

#include "CSMessage.h"
#include <maf/utils/StringifyableEnum.h>

namespace maf {
namespace messaging {

// clang-format off
MC_MAF_STRINGIFYABLE_ENUM(CSErrorCode, char,
                    InvalidParam,
                    ServerUnreachable,
                    ServiceUnavailable,
                    HandlerUnavailable,
                    ResourceUnavailable,
                    ResponseIgnored,
                    OperationFailed,
                    RequestRejected,
                    RequestTimeout,
                    Unknown
                    )
// clang-format on

class CSError : public CSMsgPayloadIF {
public:
  using ErrorCode = CSErrorCode;
  using Description = std::string;

  MAF_EXPORT CSError(Description desc = {},
                     ErrorCode code = CSErrorCode::Unknown);

  MAF_EXPORT ~CSError();
  MAF_EXPORT CSPayloadType type() const override;
  MAF_EXPORT bool equal(const CSMsgPayloadIF *other) const override;
  MAF_EXPORT CSMsgPayloadIF *clone() const override;

  MAF_EXPORT const Description &description() const;
  MAF_EXPORT void setDescription(Description description);

  MAF_EXPORT ErrorCode code() const;
  MAF_EXPORT void setCode(ErrorCode code);

  MAF_EXPORT std::string dump(int = 0) const;

private:
  struct CSErrorDataPrv *d_;
};

} // namespace messaging
} // namespace maf
