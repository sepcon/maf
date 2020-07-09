#pragma once

#include <maf/messaging/client-server/CSMsgPayloadIF.h>
#include <maf/utils/serialization/IByteStream.h>

#include <cassert>
#include <memory>

namespace maf {
namespace messaging {
namespace ipc {

class IncomingPayload : public CSMsgPayloadIF {
  using StreamType = srz::IByteStream;
  using StreamPtrType = std::shared_ptr<srz::IByteStream>;
  StreamPtrType stream_;

 public:
  IncomingPayload(StreamPtrType stream) : stream_{std::move(stream)} {}
  bool equal(const CSMsgPayloadIF *other) const override {
    if (other && (other != this)) {
      if (other->type() == CSPayloadType::IncomingData) {
        auto otherAsThis = static_cast<const IncomingPayload *>(other);
        return otherAsThis->stream() == this->stream();
      }
    }
    return false;
  }
  CSPayloadType type() const override { return CSPayloadType::IncomingData; }
  CSMsgPayloadIF *clone() const override {
    assert(stream_);
    return new IncomingPayload(std::make_shared<StreamType>(*stream_));
  }
  StreamPtrType stream() const { return stream_; }
};

}  // namespace ipc
}  // namespace messaging
}  // namespace maf
