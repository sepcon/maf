#pragma once

#include <maf/messaging/client-server/CSMsgPayloadIF.h>
#include <maf/utils/serialization/OByteStream.h>
#include <maf/utils/serialization/Serializer.h>

#include <memory>

namespace maf {
namespace messaging {
namespace ipc {
namespace local {

class MAF_EXPORT OutgoingPayload : public CSMsgPayloadIF {
 public:
  CSPayloadType type() const override { return CSPayloadType::OutgoingData; }
  virtual ~OutgoingPayload() = default;
  virtual bool serialize(srz::OByteStream &os) const = 0;
};

template <class Content>
class OutgoingPayloadT : public OutgoingPayload {
 public:
  using ContentType = std::shared_ptr<Content>;

  OutgoingPayloadT(ContentType &&content) : content_{std::move(content)} {}
  OutgoingPayloadT(const ContentType &content) : content_{content} {}

  const ContentType &content() const { return content_; }

  bool equal(const CSMsgPayloadIF *other) const override {
    if (other && other->type() == CSPayloadType::OutgoingData) {
      // assume that we always compare objects of same class
      auto otherOutgoing = static_cast<const OutgoingPayloadT *>(other);
      return this->content() && otherOutgoing->content() &&
             (*(this->content()) == *(otherOutgoing->content()));
    } else {
      return false;
    }
  }

  CSMsgPayloadIF *clone() const override {
    return new OutgoingPayloadT(content());
  }

  bool serialize(srz::OByteStream &os) const override {
    if (content_) {
      srz::SR sr(os);
      sr << *content();
      return !os.fail();
    }
    throw std::runtime_error("OutgoingPayload: empty payload");
  }

 private:
  ContentType content_;
};

}  // namespace local
}  // namespace ipc
}  // namespace messaging
}  // namespace maf
