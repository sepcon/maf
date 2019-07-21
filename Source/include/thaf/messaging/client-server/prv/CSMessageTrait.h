#pragma once

#include "thaf/messaging/client-server/interfaces/CSMessage.h"
namespace thaf {
namespace messaging {

class CSMessageTrait
{
public:
    template<class TheMessageContent>
    static OpID getOperationID() { return OpIDInvalid; }

    template <class TheMessageContent, std::enable_if_t<std::is_base_of_v<CSMessageContentBase, TheMessageContent>, bool> = true>
    static std::shared_ptr<TheMessageContent> translate(const CSMsgContentPtr&) { return nullptr; }

};


}
}
