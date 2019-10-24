#pragma once

#include "IPCMessage.h"
#include <maf/utils/debugging/Debug.h>

namespace maf {
namespace messaging {
namespace ipc {


class IPCMessageTrait
{
public:
    template<class ConcreteMessageContent>
    static OpID getOperationID()
    {
        static_assert (std::is_base_of_v<SerializableMessageContentBase, ConcreteMessageContent>,
                      "The MessageContent class must derive from SerializableMessageContentBase");
        return ConcreteMessageContent::sOperationID();
    }

    template<class ConcreteMessageContent>
    static std::shared_ptr<ConcreteMessageContent> translate(const CSMsgContentPtr& csMsgContent)
    {
        static_assert (std::is_base_of_v<SerializableMessageContentBase, ConcreteMessageContent>,
                      "The MessageContent class must derive from SerializableMessageContentBase");

        assert(csMsgContent);
        try
        {
            //Warning: to reduce the size of program, then avoid using dynamic_cast
            //We asume that the implementer will send/receive the CSMsgContentPtr as std::shared_ptr of SerializableMessageContentBase
            auto ipcContent = std::static_pointer_cast<SerializableMessageContentBase>(csMsgContent);
            auto dataCarrier = std::make_shared<ConcreteMessageContent>();
            if(!ipcContent->payload().empty())
            {
                dataCarrier->fromBytes(ipcContent->payload());
            }
            return dataCarrier;

        }
        catch(const std::exception& e)
        {
            mafErr("Could not translate message, OperationID = " << csMsgContent->operationID() << ", exception: " << e.what());
        }

        return nullptr;
    }

    template<class ConcreteMessageContent>
    static CSMsgContentPtr translate(const std::shared_ptr<ConcreteMessageContent>& msgContent)
    {
        static_assert (std::is_base_of_v<SerializableMessageContentBase, ConcreteMessageContent>,
                      "The MessageContent class must derive from SerializableMessageContentBase");
        CSMsgContentPtr content = std::static_pointer_cast<CSMessageContentBase>(msgContent);
        content->makesureTransferable();
        return content;
    }
};

}
}
}
