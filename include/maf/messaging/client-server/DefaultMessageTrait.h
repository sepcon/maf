#pragma once

#include "CSMessage.h"
#include "MessageTraitBase.h"

namespace maf {
namespace messaging {


class DefaultMessageTrait : MessageTraitBase
{
public:
    template<class CSParam>
    static OpIDConstant getOperationID()
    {
        return CSParam::operationID();
    }

    template<class CSParam>
    static std::shared_ptr<CSParam> decode(
        const CSMsgContentBasePtr& csMsgContent,
        EncodeDecodeStatus* = nullptr
        )
    {
        return std::static_pointer_cast<CSParam>(csMsgContent);
    }
    template<class CSParamAny>
    static CSMsgContentBasePtr encode(const std::shared_ptr<CSParamAny>& msgContent)
    {
        static_assert (
            std::is_base_of_v<CSMessageContentBase, CSParamAny>,
            "Must be in same hierachical tree"
            );
        return std::static_pointer_cast<CSMessageContentBase>(msgContent);
    }

};

}
}
