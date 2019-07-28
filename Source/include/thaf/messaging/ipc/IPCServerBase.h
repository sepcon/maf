#pragma once

#include "thaf/messaging/client-server/ServerBase.h"
#include "IPCTypes.h"

namespace thaf {
namespace messaging {
namespace ipc {

class IPCServerBase : public ServerBase
{
public:
    IPCServerBase();
    ~IPCServerBase() override;
    void init(IPCType type, const Address& serverAddress);
    void deinit();
    DataTransmissionErrorCode sendMessageToClient(const CSMessagePtr& msg, const Address& addr = Address::INVALID_ADDRESS)  override;
    void notifyServiceStatusToClient(ServiceID sid, Availability oldStatus, Availability newStatus) override;
     bool onIncomingMessage(const CSMessagePtr& csMsg) override;
protected:
    using RegistedClientAddresses = stl::SyncObjectM<std::set<Address>>;
    class BytesCommunicator* _communicator;
    RegistedClientAddresses _registedClAddrs;
};
} // ipc
} // messaging
} // thaf
