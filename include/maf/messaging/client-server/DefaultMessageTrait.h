#pragma once

#include "CSMessage.h"
#include "MessageTraitBase.h"

namespace maf {
namespace messaging {

class DefaultMessageTrait : public MessageTraitBase
{
public:
    template<class CSParam>
    static constexpr OpIDConst getOperationID()
    {
        return CSParam::operationID();
    }

    template<class CSParam>
    static std::shared_ptr<CSParam> decode(
        const CSMsgContentBasePtr& csMsgContent,
        CodecStatus* = nullptr
        )
    {
        static_assert (
            std::is_base_of_v<CSMessageContentBase, CSParam>,
            "Must be in same hierachical tree"
            );
        return std::static_pointer_cast<CSParam>(csMsgContent);
    }

    template<class CSParamAny>
    static CSMsgContentBasePtr encode(const std::shared_ptr<CSParamAny>& msgContent)
    {
        static_assert (
            std::is_base_of_v<CSMessageContentBase, CSParamAny>,
            "Must be in same hierachical tree"
            );
        if(msgContent)
        {
            // for inter-thread communication, the content of message should be
            // cloned instead of sharing by reference/pointer
            return CSMsgContentBasePtr{
                static_cast<const CSMessageContentBase*>(msgContent.get())->clone()
            };
        }
        else
        {
            return {};
        }
    }

};

}
}
