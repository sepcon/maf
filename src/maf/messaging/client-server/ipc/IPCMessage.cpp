#include <maf/messaging/client-server/ipc/IPCMessage.h>
#include <maf/messaging/client-server/BytesCarrier.h>
#include <maf/utils/serialization/BASerializer.h>
#include <maf/logging/Logger.h>


namespace maf { using namespace srz; using logging::Logger;
namespace messaging {
namespace ipc {


srz::ByteArray IPCMessage::toBytes()
{
    BASerializer sr;
    auto ipcContent = std::static_pointer_cast<BytesCarrier>(content());
    sr << serviceID() << operationID() << operationCode() << requestID();

    if(ipcContent)
    {
        sr << ipcContent->payload();
    }
    else
    {
        sr << srz::ByteArray{};
    }

    sr  << sourceAddress();

    return std::move(sr.mutableBytes());
}

bool IPCMessage::fromBytes(const std::shared_ptr<srz::ByteArray> &bytes) noexcept
{
    BADeserializer ds(*bytes);
    try
    {
        auto ipcContent = std::make_shared<BytesCarrier>();
        srz::ByteArray payload;
        ds >> _serviceID >> _operationID >> _operationCode >> _requestID >> payload >> _sourceAddress;
        ipcContent->setPayload(std::move(payload));
        setContent(std::move(ipcContent));
        return true;
    }
    catch (const std::exception& e)
    {
        Logger::error("Error occurred when trying to decode IPCMessage from bytes: " ,  e.what());
    }

    return false;
}


}
}
}
