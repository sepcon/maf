#pragma once

#include <maf/messaging/client-server/ServerBase.h>
#include "IPCTypes.h"
#include <set>


namespace maf {
namespace messaging {
namespace ipc {

class IPCServerBase : public ServerBase
{
public:
    IPCServerBase(IPCType ipctype);
    ~IPCServerBase() override;
    bool init(const Address& serverAddress) override;
    bool deinit() override  ;
    DataTransmissionErrorCode sendMessageToClient(const CSMessagePtr& msg, const Address& addr)  override;
    void notifyServiceStatusToClient(ServiceID sid, Availability oldStatus, Availability newStatus) override;
    bool onIncomingMessage(const CSMessagePtr& csMsg) override;
protected:
    void notifyServiceStatusToClient(const Address& clAddr, ServiceID sid, Availability oldStatus, Availability newStatus);
    using RegistedClientAddresses = nstl::Lockable<std::set<Address>>;
    class BytesCommunicator* _communicator;
    RegistedClientAddresses _registedClAddrs;
};
} // ipc
} // messaging
} // maf
