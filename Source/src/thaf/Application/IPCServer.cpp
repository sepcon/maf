#include "thaf/Application/IPCServer.h"
#include "thaf/Application/IPCMessages.h"
//#include "thaf/Messaging/IPC/IPCReplyHelper.h"

namespace thaf {
namespace app {

IPCServer::IPCServer(AppComponent *component) : _component(component)
{
}

void IPCServer::init(IPCType ipcType, Address myAddr)
{
    _serviceStub = std::make_shared<ipc::IPCServiceStub>(this);
    _serviceStub->init(ipcType, std::move(myAddr));
}

void IPCServer::startWaiting()
{
    assert(_serviceStub);
    _serviceStub->startWaitingMessages();
}

void IPCServer::stopWaiting()
{
    assert(_serviceStub);
    _serviceStub->stopWaitingMessages();
}

bool IPCServer::sendStatusUpdate(const std::shared_ptr<ipc::IPCDataCarrier>& dataCarrier)
{
    assert(_serviceStub);
    if(_serviceStub)
    {
        return _serviceStub->sendStatusUpdate(createIPCMsg(OpCode::Register, dataCarrier->getID(), dataCarrier->toBytes()));
    }
    else
    {
        thafErr("IPCServer has not been init yet!");
        return false;
    }
}

void IPCServer::deinit()
{
    if(_serviceStub)
    {
        _serviceStub->deinit();
    }
}

void IPCServer::onClientRequest(const std::shared_ptr<IPCRequestTracker> &RequestTracker)
{
    assert(_component && _serviceStub);
    _component->postMessage<IPCClientRequestMessage>(RequestTracker);
}

} // app
} // thaf
