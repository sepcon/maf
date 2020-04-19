#include <maf/messaging/client-server/CSMessage.h>

namespace maf {
namespace messaging {

CSMessage::CSMessage(ServiceID sid, OpID opID, OpCode opCode, RequestID reqID,
                     CSMsgContentBasePtr msgContent, Address sourceAddr)
    : serviceID_(std::move(sid)), operationID_(std::move(opID)),
      requestID_(std::move(reqID)), operationCode_(std::move(opCode)),
      content_(std::move(msgContent)), sourceAddress_(std::move(sourceAddr)) {}

CSMessage::~CSMessage() {}

const ServiceID &CSMessage::serviceID() const { return serviceID_; }

void CSMessage::setServiceID(ServiceID serviceID) {
  serviceID_ = std::move(serviceID);
}

const OpID &CSMessage::operationID() const { return operationID_; }

void CSMessage::setOperationID(OpID operationID) {
  operationID_ = std::move(operationID);
}

OpCode CSMessage::operationCode() const { return operationCode_; }

void CSMessage::setOperationCode(OpCode operationCode) {
  operationCode_ = std::move(operationCode);
}

RequestID CSMessage::requestID() const { return requestID_; }

void CSMessage::setRequestID(RequestID requestID) {
  requestID_ = std::move(requestID);
}

const Address &CSMessage::sourceAddress() const { return sourceAddress_; }

void CSMessage::setSourceAddress(Address sourceAddress) {
  sourceAddress_ = std::move(sourceAddress);
}

CSMsgContentBasePtr CSMessage::content() const { return content_; }

void CSMessage::setContent(CSMsgContentBasePtr content) {
  content_ = std::move(content);
}

} // namespace messaging
} // namespace maf
