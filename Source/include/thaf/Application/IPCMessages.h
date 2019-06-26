#ifndef IPCMESSAGES_H
#define IPCMESSAGES_H

#include "BasicMessages.h"
#include "thaf/Messaging/IPC/Connection.h"
#include "thaf/Messaging/IPC/IPCMessage.h"
#include "thaf/Messaging/IPC/IPCRequestTracker.h"

namespace thaf {
using namespace  messaging::ipc;
namespace app {

struct ServerConnectionStatusMsg : public ExternalMessage
{
    ServerConnectionStatusMsg(ConnectionStatus old_, ConnectionStatus new_):
        oldStatus(old_), newStatus(new_){}

    ConnectionStatus oldStatus;
    ConnectionStatus newStatus;
};


struct IPCClientRequestMessage : public ExternalMessage
{
public:
    IPCClientRequestMessage(std::shared_ptr<IPCRequestTracker> clp): _RequestTracker(std::move(clp)){}
    std::shared_ptr<IPCRequestTracker> getRequestTracker() { return _RequestTracker; }
    template<class DataCarrier, std::enable_if_t<std::is_base_of_v<IPCDataCarrier, DataCarrier>, bool> = true>
    std::shared_ptr<DataCarrier> getRequestData() const noexcept;
private:
    std::shared_ptr<IPCRequestTracker> _RequestTracker;
    friend class IPCServer;
};

template<class DataCarrier, std::enable_if_t<std::is_base_of_v<IPCDataCarrier, DataCarrier>, bool> >
std::shared_ptr<DataCarrier> IPCClientRequestMessage::getRequestData() const noexcept
{
    return _RequestTracker->getDataCarrier<DataCarrier>();
}

} // app
} // thaf

#endif // IPCMESSAGES_H
