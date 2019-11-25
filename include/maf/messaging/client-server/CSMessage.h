#pragma once

#include "CSTypes.h"
#include "Address.h"
#include <maf/export/MafExport_global.h>

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
    MAF_EXPORT  CSMessage(ServiceID sid,
        OpID opID,
        OpCode opCode,
        RequestID reqID = RequestIDInvalid,
        CSMsgContentBasePtr msgContent = nullptr,
        Address sourceAddr = {} );

    CSMessage(CSMessage&& other) = default;
    CSMessage& operator=(CSMessage&& other) = default;
    CSMessage(const CSMessage& other) = default;
    CSMessage& operator=(const CSMessage& other) = default;

    MAF_EXPORT virtual ~CSMessage();

    MAF_EXPORT const ServiceID& serviceID() const;
    MAF_EXPORT void setServiceID(ServiceID serviceID);

    MAF_EXPORT const OpID& operationID() const;
    MAF_EXPORT void setOperationID(OpID operationID);

    MAF_EXPORT OpCode operationCode() const;
    MAF_EXPORT void setOperationCode(OpCode operationCode);


    MAF_EXPORT RequestID requestID() const;
    MAF_EXPORT void setRequestID(RequestID requestID);

    MAF_EXPORT const Address& sourceAddress() const;
    MAF_EXPORT void setSourceAddress(Address sourceAddress);

    MAF_EXPORT CSMsgContentBasePtr content() const;
    MAF_EXPORT void setContent(CSMsgContentBasePtr content);

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
    MAF_EXPORT virtual ~CSMessageContentBase();
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

