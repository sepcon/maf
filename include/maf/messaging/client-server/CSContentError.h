#pragma once

#include "CSMessage.h"

namespace maf {
namespace messaging {

using ContentErrorPtr = std::shared_ptr<class CSContentError>;

class CSContentError : public CSMessageContentBase
{
public:
    using ErrorCode     = int32_t;
    using Description   = std::string;
    enum PreservedErrorCode : int32_t
    {
        InvalidParam,
        ServerUnreachable,
        ServiceUnavailable,
        HandlerUnavailable,
        StatusUnavailable,
        OperationFailed,
        RequestRejected,
        RequestTimeout,
        Unknown,
        //User defined error code must be started from Unknown + 1
    };

    MAF_EXPORT CSContentError(
        Description desc = {},
        ErrorCode code = PreservedErrorCode::Unknown
        );
    MAF_EXPORT bool equal(const CSMessageContentBase* other) const override;
    MAF_EXPORT CSMessageContentBase * clone() const override;

    MAF_EXPORT const Description& description() const;
    MAF_EXPORT void setDescription(Description description);

    MAF_EXPORT ErrorCode code() const;
    MAF_EXPORT void setCode(ErrorCode code);


private:
    Description _description;
    ErrorCode   _code = PreservedErrorCode::Unknown;
};

} // messaging
} // maf
