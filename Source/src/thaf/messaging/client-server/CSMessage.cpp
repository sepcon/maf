#include "thaf/messaging/client-server/CSMessage.h"

namespace thaf {
namespace messaging {

CSMessage::CSMessage(ServiceID tid, OpID opID, OpCode opCode, RequestID reqID, const CSMsgContentPtr msgContent, Address sourceAddr) :
    _serviceID(std::move(tid)),
    _operationID(std::move(opID)),
    _requestID(std::move(reqID)),
    _operationCode(std::move(opCode)),
    _content(std::move(msgContent)),
    _sourceAddress(std::move(sourceAddr))
{
}

CSMessage::CSMessage(CSMessage &&other)
{
    take(std::move(other));
}

CSMessage &CSMessage::operator=(CSMessage &&other)
{
    take(std::move(other));
    return *this;
}

CSMessage::~CSMessage()
{
}

ServiceID CSMessage::serviceID() const
{
    return _serviceID;
}

void CSMessage::setServiceID(ServiceID serviceID)
{
    _serviceID = std::move(serviceID);
}

OpID CSMessage::operationID() const
{
    return _operationID;
}

void CSMessage::setOperationID(OpID operationID)
{
    _operationID = std::move(operationID);
}

OpCode CSMessage::operationCode() const
{
    return _operationCode;
}

void CSMessage::setOperationCode(OpCode operationCode)
{
    _operationCode = std::move(operationCode);
}

RequestID CSMessage::requestID() const
{
    return _requestID;
}

void CSMessage::setRequestID(RequestID requestID)
{
    _requestID = std::move(requestID);
}

const Address &CSMessage::sourceAddress() const
{
    return _sourceAddress;
}

void CSMessage::setSourceAddress(Address sourceAddress)
{
    _sourceAddress = std::move(sourceAddress);
}

CSMsgContentPtr CSMessage::content() const
{
    return _content;
}

void CSMessage::setContent(CSMsgContentPtr content)
{
    _content = std::move(content);
}

void CSMessage::take(CSMessage &&other)
{
    if(&other != this)
    {
        _serviceID = std::move(other._serviceID);
        _operationID = std::move(other._operationID);
        _operationCode = std::move(other._operationCode);
        _requestID = std::move(other._requestID);
        _content = std::move(other._content);
        _sourceAddress = std::move(other._sourceAddress);
    }
}

CSMessageContentBase::~CSMessageContentBase()
{

}

void CSMessageContentBase::makesureTransferable()
{

}


} // messaging
} // thaf
