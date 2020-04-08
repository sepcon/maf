#pragma once

#include "CSMessage.h"
#include "MessageTraitBase.h"

namespace maf {
namespace messaging {

class DefaultMessageTrait : public MessageTraitBase {
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
  decode(const CSMsgContentBasePtr &csMsgContent, CodecStatus * = nullptr) {
    static_assert(std::is_base_of_v<CSMessageContentBase, Message>,
                  "Must be in same hierachical tree");
    return std::static_pointer_cast<Message>(csMsgContent);
  }

  template <class Message>
  static CSMsgContentBasePtr encode(const std::shared_ptr<Message> &msg) {
    static_assert(std::is_base_of_v<CSMessageContentBase, Message>,
                  "Must be in same hierachical tree");
    if (msg) {
      // for inter-thread communication, the content of message should be
      // cloned instead of sharing by reference/pointer
      return CSMsgContentBasePtr{
          static_cast<const CSMessageContentBase *>(msg.get())->clone()};
    } else {
      return {};
    }
  }

  template <class Message>
  static std::string dump(const std::shared_ptr<Message> &msg) {
    if (msg) {
      return msg->dump();
    } else {
      return "Null";
    }
    //    return "Not Dumpable!";
  }
};

} // namespace messaging
} // namespace maf
