#ifndef MAF_MESSAGING_CLIENT_SERVER_IPC_LOCAL_OUTGOINGMSGCONTENT_H
#define MAF_MESSAGING_CLIENT_SERVER_IPC_LOCAL_OUTGOINGMSGCONTENT_H

#include <maf/messaging/client-server/CSMessageContentBase.h>
#include <maf/utils/serialization/SerializableIF.h>
#include <memory>

namespace maf {
namespace messaging {
namespace ipc {

class OutgoingMsgContent : public CSMessageContentBase {
  using PayloadPtr = std::shared_ptr<srz::SerializableIF>;
  PayloadPtr payload_;

public:
  OutgoingMsgContent(Type type, PayloadPtr payload)
      : CSMessageContentBase(type), payload_{std::move(payload)} {}

  PayloadPtr payload() const { return payload_; }

  bool equal(const CSMessageContentBase * /*other*/) const override {
    return false;
  }
  CSMessageContentBase *clone() const override { return nullptr; }

  bool serialize(srz::OByteStream &os) const {
    if (payload_) {
      return payload_->serialize(os);
    }
    throw std::runtime_error("OutgoingMsgContent: empty payload");
  }
};

} // namespace ipc
} // namespace messaging
} // namespace maf

#endif // MAF_MESSAGING_CLIENT_SERVER_IPC_LOCAL_OUTGOINGMSGCONTENT_H
