#pragma once

#include <maf/messaging/client-server/CSMessage.h>
#include <maf/utils/cppextension/TypeInfo.h>
#include <maf/utils/serialization/ByteArray.h>

namespace maf {
namespace messaging {

using PayloadType = srz::ByteArray;
class BytesCarrier : public CSMessageContentBase {
public:
  BytesCarrier(Type type, PayloadType payload = {})
      : _payload(std::move(payload)) {
    setType(std::move(type));
  }

  bool equal(const CSMessageContentBase *other) const override {
    util::debugAssertTypesEqual(other, this);
    auto otherAsBytesCarrier = static_cast<const BytesCarrier *>(other);
    return (otherAsBytesCarrier &&
            (this->payload() == otherAsBytesCarrier->payload()));
  }

  CSMessageContentBase *clone() const override {
    return new BytesCarrier(type(), payload());
  }

  PayloadType &mutablePayload() { return _payload; }
  const PayloadType &payload() const { return _payload; }
  void setPayload(PayloadType pl) { _payload = std::move(pl); }

protected:
  PayloadType _payload;
};

} // namespace messaging
} // namespace maf
