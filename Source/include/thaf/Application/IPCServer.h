#ifndef IPCSERVER_H
#define IPCSERVER_H

#include "thaf/Messaging/IPC/IPCServiceStub.h"


namespace thaf {
using namespace messaging;
namespace messaging { namespace ipc {
class IPCDataCarrier;
}}
namespace app {

class AppComponent;


class IPCServer : public ipc::IPCServiceStubHandler
{
public:
    IPCServer(AppComponent* component);
    void init(ipc::IPCType ipcType, ipc::Address myAddr);
    void startWaiting();
    void stopWaiting();
    bool sendStatusUpdate(const std::shared_ptr<ipc::IPCDataCarrier>& dataCarrier);
    void deinit();
private:
    virtual void onClientRequest(const std::shared_ptr<ipc::IPCRequestTracker>& RequestTracker) override;

    std::shared_ptr<ipc::IPCServiceStub> _serviceStub;
    AppComponent* _component;
};
} // app
} // thaf
#endif // IPCSERVER_H
