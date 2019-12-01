#pragma once

#include "internal/cs_param.h"
#include "BytesCarrier.h"
#include "MessageTraitBase.h"
#include <maf/logging/Logger.h>

namespace maf { using logging::Logger;
namespace messaging {


class SerializableMessageTrait : public MessageTraitBase
{
public:
    template<class ContractParam>
    static constexpr OpIDConst getOperationID()
    {
        return ContractParam::operationID();
    }

    template<class cs_param_type>
    static std::shared_ptr<cs_param_type> decode(
        const CSMsgContentBasePtr& csMsgContent,
        CodecStatus* status = nullptr
        )
    {
        if(!csMsgContent)
        {
            assignCodecStatus(status, EmptyInput);
            return {};
        }

        try
        {
            using cs_param_pure_type = std::decay_t<cs_param_type>;
            // Warning: avoid using dynamic_cast RTTI
            // We asume that the implementer will send/receive the
            // CSMsgContentBasePtr as std::shared_ptr of BytesCarrier
            auto byteCarrier = std::static_pointer_cast<BytesCarrier>(
                csMsgContent
                );
            std::shared_ptr<cs_param_pure_type> dataCarrier;
            if(!byteCarrier->payload().empty())
            {
                dataCarrier = std::make_shared<cs_param_pure_type>();
                dataCarrier->fromBytes(byteCarrier->payload());
                assignCodecStatus(status, CodecStatus::Success);
            }
            else
            {
                assignCodecStatus(status, CodecStatus::EmptyInput);
            }
            return dataCarrier;

        }
        catch(const std::exception& e)
        {
            assignCodecStatus(status, CodecStatus::MalformInput);
            Logger::error(
                "Could not decode message, exception details: ",
                e.what()
                );
        }

        return nullptr;
    }

    template<class cs_param_type_base>
    static CSMsgContentBasePtr encode(const std::shared_ptr<cs_param>& param)
    {
        using serializable_param_type
            = serializable_cs_param_base<cs_param_type_base>;
        auto sbParam = std::static_pointer_cast<serializable_param_type>(param);
        auto bytesCarrier = std::make_shared<BytesCarrier>(sbParam->type());
        bytesCarrier->setPayload(
            sbParam->toBytes()
            );
        return std::move(bytesCarrier);

    }

//    static ContentErrorPtr decode(const CSMsgContentBasePtr& csMsgContent)
//    {
//        if(!csMsgContent)
//        {
//            return {};
//        }

//        auto byteCarrier = std::static_pointer_cast<BytesCarrier>(
//            csMsgContent
//            );
//        srz::BADeserializer ds(byteCarrier->payload());

//    }
//    static CSMsgContentBasePtr encode(const ContentErrorPtr& error)
//    {
//        auto bytesCarrier = std::make_shared<BytesCarrier>();
//        srz::BASerializer sr;
//        sr << error->description() << error->code();
//        bytesCarrier->setPayload(
//            std::move(sr.mutableBytes())
//            );
//        return std::move(bytesCarrier);

//    }
};

}
}
