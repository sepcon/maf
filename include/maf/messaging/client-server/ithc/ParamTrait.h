#pragma once

#include <maf/messaging/client-server/CSMessage.h>
#include <maf/messaging/client-server/ParamTraitBase.h>
#include <maf/messaging/client-server/ParamTranslatingStatus.h>

namespace maf {
namespace messaging {

class ParamTrait : public ParamTraitBase {
public:
  template <class Message>
  static std::shared_ptr<Message>
  translate(const CSMsgContentBasePtr &csMsgContent,
            TranslationStatus * = nullptr) {
    static_assert(std::is_base_of_v<CSMessageContentBase, Message>,
                  "Must be in same hierachical tree");
    return std::static_pointer_cast<Message>(csMsgContent);
  }

  template <class Message>
  static CSMsgContentBasePtr translate(const std::shared_ptr<Message> &msg) {
    static_assert(std::is_base_of_v<CSMessageContentBase, Message>,
                  "Must be in same hierachical tree");
    if (msg) {
      // for inter-thread communication, the content of message should be
      // cloned instead of sharing by reference/pointer
      auto cloned = std::make_shared<Message>(*msg);
      return std::static_pointer_cast<CSMessageContentBase>(cloned);
    } else {
      return {};
    }
  }
};

} // namespace messaging
} // namespace maf
