#include "thaf/messaging/client-server/ipc/IPCServerBase.h"
#include "thaf/messaging/client-server/ipc/BytesCommunicator.h"
#include "thaf/messaging/client-server/ServiceProviderInterface.h"

namespace thaf {
namespace messaging {
namespace ipc {

IPCServerBase::IPCServerBase() :
    _communicator(new BytesCommunicator(this))
{
}

IPCServerBase::~IPCServerBase()
{
    delete _communicator;
}

void IPCServerBase::init(IPCType type, const Address &serverAddress)
{
    _communicator->init(type, serverAddress, /*isClient = */false);
}

void IPCServerBase::deinit()
{
    _communicator->deinit();
    ServerBase::deinit();
}

DataTransmissionErrorCode IPCServerBase::sendMessageToClient(const CSMessagePtr &msg, const Address &addr)
{
    return _communicator->send(std::static_pointer_cast<IPCMessage>(msg), addr);
}

void IPCServerBase::notifyServiceStatusToClient(ServiceID sid, Availability oldStatus, Availability newStatus)
{
    if(oldStatus != newStatus)
    {
        auto serviceStatusMsg = createCSMessage<IPCMessage>(sid, newStatus == Availability::Available ? OpID_ServiceAvailable : OpID_ServiceUnavailable, OpCode::ServiceStatusUpdate);
        auto lock(_registedClAddrs.pa_lock());
        for(auto& addr : *_registedClAddrs)
        {
            _communicator->send(serviceStatusMsg, addr);;
        }
    }
}

bool IPCServerBase::onIncomingMessage(const CSMessagePtr &csMsg)
{
    if(csMsg->operationCode() == OpCode::ServiceStatusUpdate)
    {
        { //intension for release the lock of _registedClAddrs
            auto lock(_registedClAddrs.pa_lock());
            _registedClAddrs->insert(csMsg->sourceAddress());
        }
        {
            auto lock(_providers.pa_lock());
            for(auto& provider : *_providers)
            {
                notifyServiceStatusToClient(provider->serviceID(), Availability::Unavailable, Availability::Available);
            }
        }
        return true;
    }
    else
    {
        return ServerBase::onIncomingMessage(csMsg);
    }
}


} // ipc
} // messaging
} // thaf
