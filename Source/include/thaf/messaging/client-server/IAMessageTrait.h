#pragma once

#include "CSMessage.h"

namespace thaf {
namespace messaging {

class CSInAppMessageContent : public CSMessageContentBase
{
};

/**
 * @brief The IAMessageTrait class provides inteface of MessageTrait that needed by ServiceProxy/Stub classes
 * to encode and decode messages that convey the information of communication between Client and Server
 */
class IAMessageTrait
{
public:
    template<class InAppMessageContentSpecific, std::enable_if_t<std::is_base_of_v<CSMessageContentBase, InAppMessageContentSpecific>, bool> = true>
    static OpID getOperationID() { return InAppMessageContentSpecific::sOperationID(); }

    template<class InAppMessageContentSpecific, std::enable_if_t<std::is_base_of_v<CSMessageContentBase, InAppMessageContentSpecific>, bool> = true>
    static std::shared_ptr<InAppMessageContentSpecific> translate(const CSMsgContentPtr& csMsgContent)
    {
        return std::static_pointer_cast<InAppMessageContentSpecific>(csMsgContent);
    }
    template<class InAppMessageContentSpecific, std::enable_if_t<std::is_base_of_v<CSMessageContentBase, InAppMessageContentSpecific>, bool> = true>
    static CSMsgContentPtr translate(const std::shared_ptr<InAppMessageContentSpecific>& msgContent)
    {
        return std::static_pointer_cast<CSMessageContentBase>(msgContent);
    }

};

}
}
