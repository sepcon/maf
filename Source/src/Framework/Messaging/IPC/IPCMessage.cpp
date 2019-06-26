#include "headers/Framework/Messaging/IPC/Interfaces/IPCMessage.h"
#include "headers/Libs/Utils/Serialization/Serializer.h"
#include <sstream>


namespace thaf {
namespace messaging {
namespace ipc {

IPCMessage::IPCMessage() : _opCode(OpCode::Invalid), _opID(OpIDInvalid)
{

}

IPCMessage::IPCMessage(OpCode code, OpID id, const srz::ByteArray &ba) : _payload(ba), _opCode(code), _opID(id)
{
}

IPCMessage::IPCMessage(OpCode code, OpID id, srz::ByteArray &&ba) : _payload(std::move(ba)), _opCode(code), _opID(id)
{
}

IPCMessage::~IPCMessage()
{
}

void IPCMessage::setPayload(const srz::ByteArray &ba)
{
    _payload = ba;
}

void IPCMessage::setPayload(srz::ByteArray &&ba)
{
    _payload = std::move(ba);
}

const srz::ByteArray &IPCMessage::getPayload() const
{
    return _payload;
}

void IPCMessage::setOpCode(const OpCode code)
{
    _opCode = code;
}

OpCode IPCMessage::getOpCode() const
{
    return _opCode;
}

void IPCMessage::setOpID(const OpID id)
{
    _opID = id;
}

OpID IPCMessage::getOpID() const
{
    return _opID;
}

void IPCMessage::fromBytes(const srz::ByteArray &ba)
{
    srz::BADeserializer dsrz(ba);
    dsrz >> _opCode >> _opID >> static_cast<std::string&>(_payload);
}

void IPCMessage::fromBytes(const char *bytestring, size_t length)
{
    srz::ByteArray ba;
    ba.resize(length);
    memcpy(ba.firstpos(), bytestring, length);
    srz::BADeserializer dsrz(ba);
    dsrz >> _opCode >> _opID >> static_cast<std::string&>(_payload);
}

srz::ByteArray IPCMessage::toBytes()
{
    srz::BASerializer sr;
    sr << _opCode << _opID << static_cast<std::string&>(_payload);
    return std::move(sr.mutableBytes());
}

bool IPCMessage::wellformed() const
{
    return _opCode != OpCode::Invalid && _opID != OpIDInvalid;
}


}
}
}
