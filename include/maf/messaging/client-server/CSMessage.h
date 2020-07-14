#pragma once

#include <maf/export/MafExport_global.h>

#include "Address.h"
#include "CSMsgPayloadIF.h"
#include "CSTypes.h"

namespace maf {
namespace messaging {

using CSPayloadIFPtr = std::shared_ptr<CSMsgPayloadIF>;
using CSMessagePtr = std::shared_ptr<class CSMessage>;

class CSMessage {
 public:
  CSMessage() = default;
  MAF_EXPORT CSMessage(ServiceID sid, OpID opID, OpCode opCode,
                       RequestID reqID = RequestIDInvalid,
                       CSPayloadIFPtr msgContent = nullptr,
                       Address sourceAddr = {});

  CSMessage(CSMessage &&other) = default;
  CSMessage &operator=(CSMessage &&other) = default;
  CSMessage(const CSMessage &other) = default;
  CSMessage &operator=(const CSMessage &other) = default;

  MAF_EXPORT virtual ~CSMessage();

  MAF_EXPORT const ServiceID &serviceID() const;
  MAF_EXPORT void setServiceID(ServiceID serviceID);

  MAF_EXPORT const OpID &operationID() const;
  MAF_EXPORT void setOperationID(OpID operationID);

  MAF_EXPORT OpCode operationCode() const;
  MAF_EXPORT void setOperationCode(OpCode operationCode);

  MAF_EXPORT RequestID requestID() const;
  MAF_EXPORT void setRequestID(RequestID requestID);

  MAF_EXPORT const Address &sourceAddress() const;
  MAF_EXPORT void setSourceAddress(Address sourceAddress);

  MAF_EXPORT CSPayloadIFPtr payload() const;
  MAF_EXPORT void setPayload(CSPayloadIFPtr payload);

 protected:
  ServiceID serviceID_ = ServiceIDInvalid;
  OpID operationID_ = OpIDInvalid;
  RequestID requestID_ = RequestIDInvalid;
  OpCode operationCode_ = OpCode::Invalid;
  CSPayloadIFPtr payload_;
  Address sourceAddress_;
};

template <class CSMessageDerived = CSMessage>
std::shared_ptr<CSMessageDerived> createCSMessage(
    ServiceID sID, OpID opID, OpCode opCode, RequestID reqID = RequestIDInvalid,
    CSPayloadIFPtr msgContent = {}, Address sourceAddr = {}) {
  return std::make_shared<CSMessageDerived>(
      std::move(sID), std::move(opID), std::move(opCode), std::move(reqID),
      std::move(msgContent), std::move(sourceAddr));
}

}  // namespace messaging
}  // namespace maf
