#pragma once

#include "CSTypes.h"
#include "Address.h"

namespace thaf {
namespace messaging {

/// We combine 2 below values  with OpCode::ServiceStatusUpdate to inform client about server's service status(Available/UnAvailable)
constexpr OpID OpID_ServiceAvailable = 0;
constexpr OpID OpID_ServiceUnavailable = 1;

using CSMsgContentPtr = std::shared_ptr<class CSMessageContentBase>;
using CSMessagePtr = std::shared_ptr<class CSMessage>;

class CSMessage
{
public:
    CSMessage() = default;
    CSMessage(ServiceID tid, OpID opID, OpCode opCode, RequestID reqID = RequestIDInvalid,
              const CSMsgContentPtr msgContent = nullptr,
              Address sourceAddr = {} );

    CSMessage(CSMessage&& other);
    CSMessage& operator=(CSMessage&& other);

    virtual ~CSMessage();

    ServiceID serviceID() const;
    void setServiceID(ServiceID serviceID);

    OpID operationID() const;
    void setOperationID(OpID operationID);

    OpCode operationCode() const;
    void setOperationCode(OpCode operationCode);


    RequestID requestID() const;
    void setRequestID(RequestID requestID);

    const Address& sourceAddress() const;
    void setSourceAddress(Address sourceAddress);

    CSMsgContentPtr content() const;
    void setContent(CSMsgContentPtr content);

protected:
    void take(CSMessage&& other);
    ServiceID _serviceID = ServiceIDInvalid;
    OpID _operationID = OpIDInvalid;
    RequestID _requestID = RequestIDInvalid;
    OpCode _operationCode = OpCode::Invalid;
    CSMsgContentPtr _content;
    Address _sourceAddress = Address::INVALID_ADDRESS;
};

class CSMessageContentBase
{
public:
    virtual ~CSMessageContentBase();
    virtual void makesureTransferable() {}
    virtual OpID operationID() const { return OpIDInvalid; }
};

template <class CSMessageDerived = CSMessage,
            std::enable_if_t
             <
                (std::is_base_of_v<CSMessage, CSMessageDerived> || std::is_same_v<CSMessage, CSMessageDerived>) &&
                std::is_constructible_v<CSMessageDerived, ServiceID, OpID, OpCode, RequestID, const CSMsgContentPtr, Address>,
                bool
             > = true
         >
std::shared_ptr<CSMessageDerived> createCSMessage (
    ServiceID sID,
    OpID opID,
    OpCode opCode,
    RequestID reqID = RequestIDInvalid,
    const CSMsgContentPtr msgContent = nullptr,
    Address sourceAddr = {} )
{
    return std::make_shared<CSMessageDerived>(std::move(sID), std::move(opID), std::move(opCode), std::move(reqID), std::move(msgContent), std::move(sourceAddr));
}

} // messaging
} // thaf

