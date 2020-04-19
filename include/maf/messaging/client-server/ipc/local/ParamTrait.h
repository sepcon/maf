#pragma once

#include "IncomingMsgContent.h"
#include "OutgoingMsgContent.h"
#include <maf/logging/Logger.h>
#include <maf/messaging/client-server/ParamTraitBase.h>
#include <maf/messaging/client-server/ParamTranslatingStatus.h>
#include <maf/utils/Pointers.h>

namespace maf {
namespace messaging {
namespace ipc {

using util::assign_ptr;
class ParamTrait : public ParamTraitBase {
public:
  template <class Message>
  static std::shared_ptr<Message>
  translate(const CSMsgContentBasePtr &csMsgContent,
            TranslationStatus *status = nullptr) {
    if (!csMsgContent) {
      assign_ptr(status, TranslationStatus::NoSource);
      return {};
    }

    if (typeid(OutgoingMsgContent) == typeid(*(csMsgContent.get()))) {
      auto outgoingContent =
          static_cast<OutgoingMsgContent *>(csMsgContent.get());
      return std::static_pointer_cast<Message>(outgoingContent->payload());
    }

    try {
      using MessagePureT = std::decay_t<Message>;
      // Warning: avoid using dynamic_cast RTTI
      // We asume that the implementer will send/receive the
      // CSMsgContentBasePtr as std::shared_ptr of outgoingContent
      auto incomingMsg = static_cast<IncomingMsgContent*>(csMsgContent.get());
      std::shared_ptr<MessagePureT> translatedMsg;
      if (incomingMsg->stream()) {
        translatedMsg = std::make_shared<MessagePureT>();
        translatedMsg->deserialize(*incomingMsg->stream());
        assign_ptr(status, TranslationStatus::Success);
      } else {
        assign_ptr(status, TranslationStatus::NoSource);
      }
      return translatedMsg;

    } catch (const std::exception &e) {
      assign_ptr(status, TranslationStatus::DestSrcMismatch);
      MAF_LOGGER_ERROR("Could not translate message, exception details: ",
                       e.what());
    }

    return nullptr;
  }

  template <class Message>
  static CSMsgContentBasePtr translate(const std::shared_ptr<Message> &msg) {
    return std::make_shared<OutgoingMsgContent>(
        msg->type(), std::static_pointer_cast<srz::SerializableIF>(msg));
  }
};

} // namespace ipc
} // namespace messaging
} // namespace maf
