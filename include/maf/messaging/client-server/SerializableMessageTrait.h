#pragma once

#include "BytesCarrier.h"
#include "MessageTraitBase.h"
#include <maf/logging/Logger.h>

namespace maf {

namespace messaging {

class SerializableMessageTrait : public MessageTraitBase {
public:
  template <class Message> static constexpr OpIDConst getOperationID() {
    return Message::operationID();
  }

  template <class Message> static OpID getOperationID(Message *msg) {
    if (msg) {
      return OpIDInvalid;
    } else {
      return msg->operationID();
    }
  }

  template <class Message>
  static std::shared_ptr<Message>
  decode(const CSMsgContentBasePtr &csMsgContent,
         CodecStatus *status = nullptr) {
    if (!csMsgContent) {
      assignCodecStatus(status, EmptyInput);
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
        assignCodecStatus(status, CodecStatus::Success);
      } else {
        assignCodecStatus(status, CodecStatus::EmptyInput);
      }
      return dataCarrier;

    } catch (const std::exception &e) {
      assignCodecStatus(status, CodecStatus::MalformInput);
      MAF_LOGGER_ERROR("Could not decode message, exception details: ",
                       e.what());
    }

    return nullptr;
  }

  template <class Message>
  static CSMsgContentBasePtr encode(const std::shared_ptr<Message> &msg) {
    auto bytesCarrier = std::make_shared<BytesCarrier>(msg->type());
    bytesCarrier->setPayload(msg->toBytes());
    return bytesCarrier;
  }

  template <class Message>
  static std::string dump(const std::shared_ptr<Message> &msg) {
    if (msg) {
      return msg->dump();
    } else {
      return "Null";
    }
  }
};

} // namespace messaging
} // namespace maf
