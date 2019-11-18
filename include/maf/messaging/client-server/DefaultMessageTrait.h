#pragma once

#include "CSMessage.h"

namespace maf {
namespace messaging {

/**
 * @brief The DefaultMessageTrait class provides inteface of MessageTrait that needed by ServiceRequester/Stub classes
 * to encode and decode messages that convey the information for communication between Client and Server
 * DefaultMessageTrait must satisfy the requirement of Client-Server MessageTrait
 */
class DefaultMessageTrait
{
public:
    template<class CSParam>
    static OpID getOperationID()
    {
        return CSParam::operationID();
    }

    template<class CSParam>
    static std::shared_ptr<CSParam> decode(const CSMsgContentBasePtr& csMsgContent)
    {
        return std::static_pointer_cast<CSParam>(csMsgContent);
    }
    template<class CSParamAny>
    static CSMsgContentBasePtr encode(const std::shared_ptr<CSParamAny>& msgContent)
    {
        static_assert (std::is_base_of_v<CSMessageContentBase, CSParamAny>, "Must be in same hierachical tree");
        return std::static_pointer_cast<CSMessageContentBase>(msgContent);
    }

};

}
}
