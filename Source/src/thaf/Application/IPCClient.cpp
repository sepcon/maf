#include "thaf/Application/IPCClient.h"
#include "thaf/Application/AppComponent.h"
#include "thaf/Messaging/IPC/IPCServiceProxy.h"

namespace thaf {
namespace app {

IPCClient::IPCClient(AppComponent *comp) : _comp(comp)
{
}

IPCClient::~IPCClient()
{
    stop();
}

void IPCClient::init(IPCType ipcType, Address receiverAddr)
{
    _serviceProxy = std::make_unique<IPCServiceProxy>();
    _serviceProxy->init(ipcType, std::move(receiverAddr));
    _serviceProxy->startMonitoringService(this, 10);
}

void IPCClient::stop()
{
    _serviceProxy->deinit();
}

void IPCClient::onStatusChanged(ConnectionStatus oldStatus, ConnectionStatus newStatus)
{
    thafInfo("Server Status Changed: " << static_cast<int>(oldStatus) << " - " << static_cast<int>(newStatus));
    _comp->postMessage(messaging::createMessage<ServerConnectionStatusMsg>(oldStatus, newStatus));
}

void IPCClient::sendStatusChangeUnregister(RegID regID)
{
    _serviceProxy->sendStatusChangeUnregister(std::move(regID));
}


} // app
} // thaf
