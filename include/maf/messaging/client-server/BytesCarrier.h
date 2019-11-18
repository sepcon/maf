#pragma once

#include <maf/messaging/client-server/CSMessage.h>
#include <maf/utils/serialization/ByteArray.h>
#include <maf/utils/cppextension/TypeInfo.h>

namespace maf {
namespace messaging {

using PayloadType = srz::ByteArray;
class BytesCarrier : public CSMessageContentBase
{
public:
    bool equal(const CSMessageContentBase *other) override
    {
        util::debugAssertTypesEqual(other, this);
        auto otherAsBytesCarrier = static_cast<const BytesCarrier*>(other);
        return (
            otherAsBytesCarrier &&
            (this->payload() == otherAsBytesCarrier->payload())
            );
    }

    PayloadType& mutablePayload() { return _payload; }
    const PayloadType& payload() const { return _payload; }
    void setPayload(PayloadType pl) { _payload = std::move(pl); }

protected:
    PayloadType _payload;
};

}
}
