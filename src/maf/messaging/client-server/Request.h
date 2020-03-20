#pragma once

#include <maf/messaging/client-server/RequestIF.h>
#include <maf/messaging/client-server/internal/CSShared.h>
#include <maf/patterns/Patterns.h>
#include <mutex>

namespace maf {
namespace messaging {

class ServiceProviderIF;

class Request : public RequestIF, public pattern::Unasignable {
  friend struct ServiceProviderImpl;

  std::shared_ptr<CSMessage> _csMsg;
  std::weak_ptr<ServiceProviderIF> _svStub;
  AbortRequestCallback _abortCallback;
  mutable std::mutex _mutex;
  bool _valid;

  Request(std::shared_ptr<CSMessage> csMsg,
          std::weak_ptr<ServiceProviderIF> svStub);
  bool invalidate();
  AbortRequestCallback getAbortCallback();
  ActionCallStatus sendMsgBackToClient();
  void setOperationCode(OpCode opCode);

public:
  OpCode getOperationCode() const override;
  const OpID &getOperationID() const override;
  RequestID getRequestID() const override;
  bool valid() const override;
  ActionCallStatus respond(const CSMsgContentBasePtr &answer) override;
  CSMsgContentBasePtr getInput() override;
  void setAbortRequestHandler(AbortRequestCallback abortCallback) override;
};

} // namespace messaging
} // namespace maf
