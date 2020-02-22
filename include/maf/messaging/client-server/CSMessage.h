#pragma once

#include "CSTypes.h"
#include "Address.h"
#include <maf/export/MafExport_global.h>

namespace maf {
namespace messaging {

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
                         Address sourceAddr = {});

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
    ServiceID           _serviceID      = ServiceIDInvalid;
    OpID                _operationID    = OpIDInvalid;
    RequestID           _requestID      = RequestIDInvalid;
    OpCode              _operationCode  = OpCode::Invalid;
    CSMsgContentBasePtr _content;
    Address             _sourceAddress;
};

class CSMessageContentBase
{
public:
    enum class Type : char
    {
        Data,
        Error,
        NA
    };

    MAF_EXPORT virtual ~CSMessageContentBase();
    MAF_EXPORT Type type() const;
    MAF_EXPORT void setType(Type t);
    virtual bool equal(const CSMessageContentBase* other) const = 0;
    virtual CSMessageContentBase* clone() const = 0;

private:
    Type             _type   = Type::NA;
};

template <class CSMessageDerived = CSMessage>
std::shared_ptr<CSMessageDerived> createCSMessage (
    ServiceID sID,
    OpID opID,
    OpCode opCode,
    RequestID reqID = RequestIDInvalid,
    CSMsgContentBasePtr msgContent = {},
    Address sourceAddr = {})
{
    return std::make_shared<CSMessageDerived>(
        std::move(sID),
        std::move(opID),
        std::move(opCode),
        std::move(reqID),
        std::move(msgContent),
        std::move(sourceAddr)
        );
}

} // messaging
} // maf

