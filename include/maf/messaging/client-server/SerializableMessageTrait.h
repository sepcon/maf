#pragma once

#include "internal/cs_param.h"
#include "BytesCarrier.h"
#include <maf/logging/Logger.h>

namespace maf { using logging::Logger;
namespace messaging {
namespace ipc {


class SerializableMessageTrait
{
public:
    template<class ContractParam>
    static OpID getOperationID()
    {
        return ContractParam::operationID();
    }

    template<class cs_param_type>
    static std::shared_ptr<cs_param_type> decode(const CSMsgContentBasePtr& csMsgContent)
    {
        assert(csMsgContent);
        try
        {
            using cs_param_pure_type = std::decay_t<cs_param_type>;
            //Warning: to reduce the size of program, then avoid using dynamic_cast
            //We asume that the implementer will send/receive the CSMsgContentBasePtr as std::shared_ptr of BytesCarrier
            auto byteCarrier = std::static_pointer_cast<BytesCarrier>(csMsgContent);
            std::shared_ptr<cs_param_pure_type> dataCarrier;
            if(!byteCarrier->payload().empty())
            {
                dataCarrier = std::make_shared<cs_param_pure_type>();
                dataCarrier->fromBytes(byteCarrier->payload());
            }
            return dataCarrier;

        }
        catch(const std::exception& e)
        {
            Logger::error("Could not decode message, exception details: " ,  e.what());
        }

        return nullptr;
    }

    template<class cs_param_type_base>
    static CSMsgContentBasePtr encode(const std::shared_ptr<cs_param>& param)
    {
        using serializable_param_type = serializable_cs_param_base<cs_param_type_base>;
        auto bytesCarrier = std::make_shared<BytesCarrier>();
        bytesCarrier->setPayload(
            std::static_pointer_cast<serializable_param_type>(param)->toBytes()
            );
        return std::move(bytesCarrier);

    }
};

}
}
}
