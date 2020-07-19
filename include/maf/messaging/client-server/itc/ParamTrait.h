#pragma once

#include "Payload.h"
#include <maf/messaging/client-server/CSMessage.h>
#include <maf/messaging/client-server/ParamTraitBase.h>
#include <maf/messaging/client-server/ParamTranslatingStatus.h>

namespace maf {
namespace messaging {
namespace itc {

class ParamTrait : public ParamTraitBase {
public:
  template <class Content>
  static std::shared_ptr<Content> translate(const CSPayloadIFPtr &csMsgContent,
                                            TranslationStatus * = nullptr) {
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
      return std::make_shared<Payload<Content>>(
          CSPayloadType::OutgoingData, std::make_shared<Content>(*msg));
    } else {
      return {};
    }
  }
};

} // namespace itc
} // namespace messaging
} // namespace maf
