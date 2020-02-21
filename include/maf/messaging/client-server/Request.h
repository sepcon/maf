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

    std::shared_ptr<CSMessage>                      _csMsg;
    std::weak_ptr<ServiceProviderInterface>         _svStub;
    AbortRequestCallback                            _abortCallback;
    mutable std::mutex                              _mutex;
    bool                                            _valid;

    Request(
        std::shared_ptr<CSMessage> csMsg,
        std::weak_ptr<ServiceProviderInterface> svStub
        );
    bool invalidate();
    AbortRequestCallback getAbortCallback();
    ActionCallStatus sendMsgBackToClient();
    void setOperationCode(OpCode opCode);

public:
    OpCode getOperationCode() const override;
    const OpID& getOperationID() const override;
    RequestID getRequestID() const override;
    bool valid() const override;
    ActionCallStatus respond(const CSMsgContentBasePtr& answer) override;
    CSMsgContentBasePtr getInput() override;
    void setAbortRequestHandler(AbortRequestCallback abortCallback) override;


};

} // messaging
} // maf
