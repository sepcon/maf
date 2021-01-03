#pragma once

#include <maf/messaging/client-server/CSMessage.h>
#include <maf/messaging/client-server/ParamTraitBase.h>
#include <maf/messaging/client-server/ParamTranslatingStatus.h>
#include <maf/utils/Pointers.h>

#include "Payload.h"

namespace maf {
namespace messaging {
namespace itc {

class ParamTrait : public ParamTraitBase {
 public:
  template <class Content>
  static std::shared_ptr<Content> translate(
      const CSPayloadIFPtr &csMsgContent, TranslationStatus *status = nullptr) {
    util::assign_ptr(status, TranslationStatus::Success);
    if (csMsgContent) {
      return static_cast<Payload<Content> *>(csMsgContent.get())->content();
    } else {
      return {};
    }
  }

  template <class Content>
  static CSPayloadIFPtr translate(const std::shared_ptr<Content> &msg) {
    if (msg) {
      // for inter-thread communication, the content of message should be
      // cloned instead of shared by reference/pointer
      return std::make_shared<Payload<Content>>(CSPayloadType::OutgoingData,
                                                msg);
    } else {
      return {};
    }
  }

  template <class Message>
  static std::string dump(const std::shared_ptr<Message> &msg) {
    return typeid(Message).name();
  }
};

}  // namespace itc
}  // namespace messaging
}  // namespace maf
