#pragma once

#include "CSMessage.h"
#include "ParamTraitBase.h"
#include "ParamTranslatingStatus.h"

namespace maf {
namespace messaging {

class DefaultParamTrait : public ParamTraitBase {
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
      //      return CSMsgContentBasePtr{
      //          static_cast<const CSMessageContentBase
      //          *>(msg.get())->clone()};
    } else {
      return {};
    }
  }
};

} // namespace messaging
} // namespace maf
