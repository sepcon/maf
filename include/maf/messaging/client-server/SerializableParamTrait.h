#pragma once

#include "BytesCarrier.h"
#include "ParamTranslatingStatus.h"
#include "ParamTraitBase.h"
#include <maf/utils/Pointers.h>
#include <maf/logging/Logger.h>

namespace maf {
namespace messaging {

using util::assign_ptr;
class SerializableParamTrait : public ParamTraitBase {
public:
  template <class Message>
  static std::shared_ptr<Message>
  translate(const CSMsgContentBasePtr &csMsgContent,
         TranslationStatus *status = nullptr) {
    if (!csMsgContent) {
      assign_ptr(status, TranslationStatus::NoSource);
      return {};
    }

    try {
      using MessagePureT = std::decay_t<Message>;
      // Warning: avoid using dynamic_cast RTTI
      // We asume that the implementer will send/receive the
      // CSMsgContentBasePtr as std::shared_ptr of BytesCarrier
      auto byteCarrier = std::static_pointer_cast<BytesCarrier>(csMsgContent);
      std::shared_ptr<MessagePureT> dataCarrier;
      if (!byteCarrier->payload().empty()) {
        dataCarrier = std::make_shared<MessagePureT>();
        dataCarrier->fromBytes(byteCarrier->payload());
        assign_ptr(status, TranslationStatus::Success);
      } else {
        assign_ptr(status, TranslationStatus::NoSource);
      }
      return dataCarrier;

    } catch (const std::exception &e) {
      assign_ptr(status, TranslationStatus::DestSrcMismatch);
      MAF_LOGGER_ERROR("Could not translate message, exception details: ",
                       e.what());
    }

    return nullptr;
  }

  template <class Message>
  static CSMsgContentBasePtr translate(const std::shared_ptr<Message> &msg) {
    auto bytesCarrier = std::make_shared<BytesCarrier>(msg->type());
    bytesCarrier->setPayload(msg->toBytes());
    return bytesCarrier;
  }
};

} // namespace messaging
} // namespace maf
