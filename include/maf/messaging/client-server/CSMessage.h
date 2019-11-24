#pragma once

#include "CSTypes.h"
#include "Address.h"

namespace maf {
namespace messaging {

/// We combine 2 below values  with OpCode::ServiceStatusUpdate to inform client about server's service status(Available/UnAvailable)
constexpr OpIDConstant OpID_ServiceAvailable    = "OpID_ServiceAvailable";
constexpr OpIDConstant OpID_ServiceUnavailable  = "OpID_ServiceUnavailable";

using CSMsgContentBasePtr = std::shared_ptr<class CSMessageContentBase>;
using CSMessagePtr = std::shared_ptr<class CSMessage>;

class CSMessage
{
public:
    CSMessage() = default;
    CSMessage(
        ServiceID tid,
        OpID opID,
        OpCode opCode,
        RequestID reqID = RequestIDInvalid,
        CSMsgContentBasePtr msgContent = nullptr,
        Address sourceAddr = {} );

    CSMessage(CSMessage&& other) = default;
    CSMessage& operator=(CSMessage&& other) = default;
    CSMessage(const CSMessage& other) = default;
    CSMessage& operator=(const CSMessage& other) = default;

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

    CSMsgContentBasePtr content() const;
    void setContent(CSMsgContentBasePtr content);

protected:
    ServiceID _serviceID = ServiceIDInvalid;
    OpID _operationID = OpIDInvalid;
    RequestID _requestID = RequestIDInvalid;
    OpCode _operationCode = OpCode::Invalid;
    CSMsgContentBasePtr _content;
    Address _sourceAddress = Address::INVALID_ADDRESS;
};

class CSMessageContentBase
{
public:
    virtual ~CSMessageContentBase();
    virtual bool equal(const CSMessageContentBase* other) = 0;
};

template <class CSMessageDerived = CSMessage,
            std::enable_if_t
             <
                (std::is_base_of_v<CSMessage, CSMessageDerived> || std::is_same_v<CSMessage, CSMessageDerived>) &&
                std::is_constructible_v<CSMessageDerived, ServiceID, OpID, OpCode, RequestID, CSMsgContentBasePtr, Address>,
                bool
             > = true
         >
std::shared_ptr<CSMessageDerived> createCSMessage (
    ServiceID sID,
    OpID opID,
    OpCode opCode,
    RequestID reqID = RequestIDInvalid,
    CSMsgContentBasePtr msgContent = {},
    Address sourceAddr = {} )
{
    return std::make_shared<CSMessageDerived>(std::move(sID), std::move(opID), std::move(opCode), std::move(reqID), std::move(msgContent), std::move(sourceAddr));
}

} // messaging
} // maf

