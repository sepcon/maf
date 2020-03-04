#pragma once

#include <maf/messaging/client-server/ipc/IPCSender.h>
#include "SocketShared.h"

namespace maf {
namespace messaging {
namespace ipc {


class  LocalIPCSenderImpl: public IPCSender
{
public:
    LocalIPCSenderImpl();
    ~LocalIPCSenderImpl() override;
    ActionCallStatus send(const maf::srz::ByteArray &payload, const Address& destination) override;
    bool initConnection(const Address&receiverAddr) override;
    Availability checkReceiverStatus() const override;
    const Address& receiverAddress() const override;

private:
    Address _myReceiverAddr;
    std::unique_ptr<SocketPath> _myReceiverSocketPath;
    std::unique_ptr<sockaddr_un> _myReceiverSockAddr;
};

}}}

