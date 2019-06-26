#ifndef IPCMESSAGE_H
#define IPCMESSAGE_H

#include "thaf/Utils/Serialization/ByteArray.h"
#include "thaf/Utils/Serialization/SerializableObject.h"
#include "thaf/Utils/Debugging/Debug.h"
#include "Address.h"
#include "ClientServerContract.h"
#include <functional>

namespace thaf {
namespace messaging {
namespace ipc {

mc_sbClass(IPCMessage)
mc_sbProperties
    (
        (OpCode, operation_code, OpCode::Invalid),
        (OpID, operation_id, static_cast<OpID>(OpIDInvalid)),
        (srz::ByteArray, payload),
        (Address, sender_addr),
        (uint32_t, request_id, static_cast<uint32_t>(-1))
    )

public:
    bool wellformed() const
    {
        return get_operation_id() != OpIDInvalid && get_operation_code() != OpCode::Invalid;
    }

mc_sbClass_end(IPCMessage)


inline std::shared_ptr<IPCMessage> createIPCMsg(OpCode code, OpID id, srz::ByteArray bytes = {}, Address addr = {}, uint32_t requestID = static_cast<uint32_t>(-1))
{
    return std::make_shared<IPCMessage>(code, id, std::move(bytes), std::move(addr), requestID);
}

template<class SerializableObject>
std::shared_ptr<IPCMessage> createIPCMsg(OpCode code, OpID id, const SerializableObject& obj = {}, Address addr = {}, uint32_t requestID = static_cast<uint32_t>(-1))
{
    srz::BASerializer sr;
    sr << obj;
    return std::make_shared<IPCMessage>(code, id, std::move(sr.mutableBytes()), std::move(addr), requestID);
}

template<class SerializableObject>
std::shared_ptr<SerializableObject> getPayloadObject(const std::shared_ptr<IPCMessage>& ipcMsg)
{
    srz::BADeserializer ds(ipcMsg->get_payload());
    std::shared_ptr<SerializableObject> payloadObject;
    try
    {
        ds >> payloadObject;
    }
    catch (const std::exception& e)
    {
        thafErr("Could not decode the incomming payload, seems that having mismatch between client and server contract" <<
               "\nException content: " << e.what());
    }
    return payloadObject;
}

}
}
}
#endif // IPCMESSAGE_H

