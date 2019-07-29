#pragma once

#include "IPCMessage.h"
#include "thaf/utils/debugging/Debug.h"

namespace thaf {
namespace messaging {
namespace ipc {


class IPCMessageTrait
{
public:
    template<class IPCMessageContent, std::enable_if_t<std::is_base_of_v<SerializableMessageContentBase, IPCMessageContent>, bool> = true>
    static OpID getOperationID() { return IPCMessageContent::sOperationID(); }

    template<class IPCMessageContent, std::enable_if_t<std::is_base_of_v<SerializableMessageContentBase, IPCMessageContent>, bool> = true>
    static std::shared_ptr<IPCMessageContent> translate(const CSMsgContentPtr& csMsgContent)
    {
        assert(csMsgContent);
        try
        {
            //Warning: to reduce the size of program, then avoid using dynamic_cast
            //We asume that the implementer will send/receive the CSMsgContentPtr as std::shared_ptr of SerializableMessageContentBase
            auto ipcContent = std::static_pointer_cast<SerializableMessageContentBase>(csMsgContent);
            auto dataCarrier = std::make_shared<IPCMessageContent>();
            dataCarrier->fromBytes(ipcContent->payload());
            return dataCarrier;

        }
        catch(const std::exception& e)
        {
            thafErr("Could not translate message, OperationID = " << csMsgContent->operationID() << ", exception: " << e.what());
        }

        return nullptr;
    }

    template<class IPCMessageContent, std::enable_if_t<std::is_base_of_v<SerializableMessageContentBase, IPCMessageContent>, bool> = true>
    static CSMsgContentPtr translate(const std::shared_ptr<IPCMessageContent>& msgContent)
    {
        CSMsgContentPtr content = std::static_pointer_cast<CSMessageContentBase>(msgContent);
        content->makesureTransferable();
        return content;
    }
};

}
}
}
