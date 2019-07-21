#include "thaf/messaging/ipc/IPCMessage.h"
#include "thaf/utils/serialization/Serializer.h"
#include <thaf/utils/debugging/Debug.h>


namespace thaf { using namespace srz;
namespace messaging {
namespace ipc {


srz::ByteArray IPCMessage::toBytes() const noexcept
{
    BASerializer sr;
    auto ipcContent = std::static_pointer_cast<IPCMessageContentBase>(content());
    srz::ByteArray payload = "";
    if(ipcContent)
    {
        payload = ipcContent->payload();
    }

    sr << serviceID() << operationID() << operationCode() << requestID() << static_cast<const std::string&>(payload) << sourceAddress();

    return std::move(sr.mutableBytes());
}

bool IPCMessage::fromBytes(const std::shared_ptr<srz::ByteArray> &bytes) noexcept
{
    BADeserializer ds(*bytes);
    try
    {
        auto ipcContent = std::make_shared<IPCMessageContentBase>();
        ds >> _serviceID >> _operationID >> _operationCode >> _requestID >> ipcContent->payload() >> _sourceAddress;
        setContent(std::static_pointer_cast<CSMessageContentBase>(ipcContent));
        return true;
    }
    catch (const std::exception& e)
    {
        thafErr("Error occurred when trying to decode IPCMessage from bytes: " << e.what());
    }

    return false;
}


}
}
}
