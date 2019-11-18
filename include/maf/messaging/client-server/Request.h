#pragma once

#include "RequestInterface.h"
#include "internal/CSShared.h"
#include <maf/patterns/Patterns.h>
#include <mutex>

namespace maf {
namespace messaging {

class ServiceProviderInterface;

class Request : public pattern::Unasignable, public RequestInterface
{
    friend struct ServiceProviderImpl;

private:
    std::shared_ptr<CSMessage>                      _csMsg;
    std::weak_ptr<ServiceProviderInterface>         _svStub;
    RequestAbortedCallback                          _abortCallback;
    mutable std::mutex                              _mutex;
    bool                                            _valid;

private:
    Request(std::shared_ptr<CSMessage> csMsg, std::weak_ptr<ServiceProviderInterface> svStub);
    bool invalidate();
    RequestAbortedCallback getAbortCallback();
    ActionCallStatus sendMsgBackToClient();
    void setOperationCode(OpCode opCode);

public:
    OpCode getOperationCode() const override;
    OpID getOperationID() const override;
    RequestID getRequestID() const override;
    bool valid() const override;
    ActionCallStatus respond(const CSMsgContentBasePtr& answer) override;
    CSMsgContentBasePtr getRequestContent() override;
    void onAbortRequest(RequestAbortedCallback abortCallback) override;


};

} // messaging
} // maf
