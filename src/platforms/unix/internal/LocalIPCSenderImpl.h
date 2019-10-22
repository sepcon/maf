#pragma once

#include <maf/messaging/client-server/ipc/IPCSender.h>

namespace maf {
namespace messaging {
namespace ipc {


class  LocalIPCSenderImpl: public IPCSender
{
public:
    LocalIPCSenderImpl();
    ~LocalIPCSenderImpl() override;
    DataTransmissionErrorCode send(const maf::srz::ByteArray &ba, const Address& destination = Address::INVALID_ADDRESS) override;
    void initConnection(const Address&) override;
    Availability checkReceiverStatus() const override;
    const Address& receiverAddress() const override;
};

}}}

