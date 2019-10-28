#pragma once

#include "CSMessage.h"

namespace maf {
namespace messaging {

/**
 * @brief The IAMessageTrait class provides inteface of MessageTrait that needed by ServiceProxy/Stub classes
 * to encode and decode messages that convey the information for communication between Client and Server
 * IAMessageTrait must satisfy the requirement of Client-Server MessageTrait
 */
class IAMessageTrait
{
public:
    template<class IAConcreteMessageContent, std::enable_if_t<std::is_base_of_v<CSMessageContentBase, IAConcreteMessageContent>, bool> = true>
    static OpID getOperationID()
    {
        static_assert (
            std::is_base_of_v<CSMessageContentBase, IAConcreteMessageContent>,
            "the message content must derive from CSMessageContentBase and satisfy requirement of MessageTrait template"
            );
        return IAConcreteMessageContent::sOperationID(); }

    template<class IAConcreteMessageContent, std::enable_if_t<std::is_base_of_v<CSMessageContentBase, IAConcreteMessageContent>, bool> = true>
    static std::shared_ptr<IAConcreteMessageContent> translate(const CSMsgContentPtr& csMsgContent)
    {
        static_assert (
            std::is_base_of_v<CSMessageContentBase, IAConcreteMessageContent>,
            "the message content must derive from CSMessageContentBase and satisfy requirement of MessageTrait template"
            );
        return std::static_pointer_cast<IAConcreteMessageContent>(csMsgContent);
    }
    template<class IAConcreteMessageContent, std::enable_if_t<std::is_base_of_v<CSMessageContentBase, IAConcreteMessageContent>, bool> = true>
    static CSMsgContentPtr translate(const std::shared_ptr<IAConcreteMessageContent>& msgContent)
    {
        static_assert (
            std::is_base_of_v<CSMessageContentBase, IAConcreteMessageContent>,
            "the message content must derive from CSMessageContentBase and satisfy requirement of MessageTrait template"
            );
        return std::static_pointer_cast<CSMessageContentBase>(msgContent);
    }

};

}
}
