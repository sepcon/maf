#pragma once

#include "ServiceProviderShared.h"

namespace maf {
namespace messaging {

class RequestInterface
{
public:
    virtual ~RequestInterface() = default;
    virtual OpCode getOperationCode() const = 0;
    virtual const OpID& getOperationID() const = 0;
    virtual RequestID getRequestID() const = 0;
    virtual bool valid() const = 0;
    virtual ActionCallStatus respond(const CSMsgContentBasePtr& answer) = 0;
    virtual CSMsgContentBasePtr getInput() = 0;
    virtual void setAbortRequestHandler(AbortRequestCallback abortCallback) = 0;
};

} // messaging
} // maf
