#pragma once

#include <maf/messaging/client-server/CSMsgPayloadIF.h>
#include <maf/messaging/client-server/cs_param.h>
#include <memory>

namespace maf {
namespace messaging {
namespace itc {

template <class Content> class Payload : public CSMsgPayloadIF {
public:
  using ContentType = std::shared_ptr<Content>;

  Payload(CSPayloadType type, ContentType payload = {})
      : content_{std::move(payload)}, type_{type} {}

  CSPayloadType type() const override { return type_; }
  const ContentType &content() const { return content_; }
  void setContent(ContentType &&content) { content_ = std::move(content); }
  void setContent(const ContentType &content) { content_ = content; }

  bool equal(const CSMsgPayloadIF *other) const override {
    if (other) {
      if ((other != this) && (other->type() == this->type())) {
        auto &thisContent = this->content();
        auto &otherContent = static_cast<const Payload *>(other)->content();
        if (thisContent && otherContent) {
          return *thisContent == *otherContent;
        }
      }
    }
    return false;
  }

  CSMsgPayloadIF *clone() const override {
    return new Payload(type_, content_);
  }

private:
  ContentType content_;
  CSPayloadType type_ = CSPayloadType::NA;
};

} // namespace itc
} // namespace messaging
} // namespace maf
