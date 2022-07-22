#include "Request.h"

#include <maf/logging/Logger.h>
#include <maf/messaging/client-server/CSError.h>
#include <maf/messaging/client-server/ServiceProviderIF.h>

namespace maf {
namespace messaging {

Request::Request(std::shared_ptr<CSMessage> csMsg,
                 std::weak_ptr<ServiceProviderIF> svStub)
    : _csMsg(std::move(csMsg)), _svStub(std::move(svStub)), _valid(true) {}

AbortRequestCallback Request::getAbortCallback() {
  std::lock_guard lock(_mutex);
  return _abortCallback;
}

OpCode Request::getOperationCode() const {
  std::lock_guard lock(_mutex);
  return _csMsg->operationCode();
}

const OpID &Request::getOperationID() const {
  std::lock_guard lock(_mutex);
  return _csMsg->operationID();
}

bool Request::valid() const {
  std::lock_guard lock(_mutex);
  return _valid;
}

ActionCallStatus Request::respond(const CSPayloadIFPtr &answer) {
  return reply(answer, OpCode::Request);
}

ActionCallStatus Request::update(const CSPayloadIFPtr &answer) {
  return reply(answer, OpCode::PartialRequestUpdate);
}

ActionCallStatus Request::reply(const CSPayloadIFPtr &answer, OpCode code) {
  std::unique_lock lock(_mutex);
  if (_valid) {
    // set invalid here to avoid others from sending another reponse
    // while sending the current one is being sent to client

    auto replyMsg = std::make_shared<CSMessage>(*_csMsg);
    replyMsg->setPayload(answer);
    replyMsg->setOperationCode(code);

    if (auto stub = _svStub.lock()) {
      // unlock to avoid deadlock when stub tries to invalidate this request
      // and to avoid other thread from waiting for lock during message sending
      lock.unlock();
      return stub->respondToRequest(replyMsg);
    } else {
      return ActionCallStatus::ReceiverUnavailable;
    }

  } else {
    MAF_LOGGER_ERROR("Request is no longer valid, might be the operation id [",
                     _csMsg->operationID(), "]");
    return ActionCallStatus::InvalidCall;
  }
}

CSPayloadIFPtr Request::getInput() {
  std::lock_guard lock(_mutex);
  return _csMsg->payload();
}

void Request::onAborted(AbortRequestCallback abortCallback) {
  std::lock_guard lock(_mutex);
  if (!_valid) {
    abortCallback();
  } else {
    _abortCallback = std::move(abortCallback);
  }
}

bool Request::invalidate() {
  std::lock_guard lock(_mutex);
  if (_valid) {
    _valid = false;
    _svStub.reset();
    return true;
  } else {
    return false;
  }
}

RequestID Request::getRequestID() const {
  std::lock_guard lock(_mutex);
  return _csMsg->requestID();
}

ActionCallStatus Request::sendMsgBackToClient() {
  if (auto stub = _svStub.lock()) {
    return stub->respondToRequest(_csMsg);
  } else {
    return ActionCallStatus::ReceiverUnavailable;
  }
}

void Request::setOperationCode(OpCode opCode) {
  std::lock_guard lock(_mutex);
  _csMsg->setOperationCode(opCode);
}

}  // namespace messaging
}  // namespace maf
