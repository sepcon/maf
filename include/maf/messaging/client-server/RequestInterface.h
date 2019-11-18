#pragma once

#include "internal/CSShared.h"

namespace maf {
namespace messaging {

class RequestInterface
{
public:
    //  intended to not having destructor
    //  virtual ~RequestInterface() = 0;
    virtual OpCode getOperationCode() const = 0;
    virtual OpID getOperationID() const = 0;
    virtual RequestID getRequestID() const = 0;
    virtual bool valid() const = 0;
    virtual ActionCallStatus respond(const CSMsgContentBasePtr& answer) = 0;
    virtual CSMsgContentBasePtr getRequestContent() = 0;
    virtual void onAbortRequest(RequestAbortedCallback abortCallback) = 0;
};

} // messaging
} // maf
