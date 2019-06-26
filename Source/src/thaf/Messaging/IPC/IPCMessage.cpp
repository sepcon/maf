//#include "thaf/Messaging/IPC/IPCMessage.h"
//#include "thaf/Utils/Serialization/Serializer.h"
//#include <sstream>


//namespace thaf {
//namespace messaging {
//namespace ipc {

//IPCMessage::IPCMessage() : _opID(OpIDInvalid), _opCode(OpCode::Invalid)
//{

//}

//IPCMessage::IPCMessage(OpCode code, OpID id, srz::ByteArray ba, Address sender_addr) :
//    _sender_addr(std::move(sender_addr)),  _payload(std::move(ba)), _opID(id), _opCode(code)
//{
//}


//void IPCMessage::setPayload(const srz::ByteArray &ba)
//{
//    _payload = ba;
//}

//void IPCMessage::setPayload(srz::ByteArray &&ba)
//{
//    _payload = std::move(ba);
//}

//const srz::ByteArray &IPCMessage::getPayload() const
//{
//    return _payload;
//}

//void IPCMessage::setOpCode(const OpCode code)
//{
//    _opCode = code;
//}

//OpCode IPCMessage::getOpCode() const
//{
//    return _opCode;
//}

//void IPCMessage::setOpID(const OpID id)
//{
//    _opID = id;
//}

//OpID IPCMessage::getOpID() const
//{
//    return _opID;
//}

//void IPCMessage::serialize(srz::Serializer &sr) const
//{
//    sr << _opCode << _opID << _sender_addr << _payload;
//}

//void IPCMessage::deserialize(srz::Deserializer &dsr)
//{
//    dsr >> _opCode >> _opID >> _sender_addr >> _payload;
//}

//bool IPCMessage::wellformed() const
//{
//    return _opCode != OpCode::Invalid && _opID != OpIDInvalid;
//}

//const Address &IPCMessage::getSenderAddr() const
//{
//    return _sender_addr;
//}

//void IPCMessage::setSenderAddr(Address sender_addr)
//{
//    _sender_addr = std::move(sender_addr);
//}


//}
//}
//}
