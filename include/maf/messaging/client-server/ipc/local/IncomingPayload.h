#pragma once

#include <maf/messaging/client-server/CSMsgPayloadIF.h>
#include <maf/utils/serialization/IByteStream.h>

#include <cassert>
#include <memory>

namespace maf {
namespace messaging {
namespace ipc {
namespace local {

class IncomingPayload : public CSMsgPayloadIF {
  using StreamType = srz::IByteStream;
  using StreamPtrType = std::unique_ptr<srz::IByteStream>;
  StreamPtrType stream_;

 public:
  IncomingPayload(StreamPtrType stream) : stream_{std::move(stream)} {}
  bool equal(const CSMsgPayloadIF *other) const override {
    if (other && (other != this)) {
      if (other->type() == CSPayloadType::IncomingData) {
        auto otherAsThis = static_cast<const IncomingPayload *>(other);
        // Don't compare content of stream
        return otherAsThis->stream() == this->stream();
      }
    }
    return false;
  }
  CSPayloadType type() const override { return CSPayloadType::IncomingData; }
  CSMsgPayloadIF *clone() const override {
    assert(stream_);
    return new IncomingPayload(std::make_unique<StreamType>(*stream_));
  }
  const StreamPtrType &stream() const { return stream_; }
};

}  // namespace local
}  // namespace ipc
}  // namespace messaging
}  // namespace maf
