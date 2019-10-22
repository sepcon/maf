#include <maf/messaging/client-server/ipc/IPCServerBase.h>
#include <maf/messaging/client-server/ipc/BytesCommunicator.h>
#include <maf/messaging/client-server/ServiceProviderInterface.h>

namespace maf {
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
        std::lock_guard lock(_registedClAddrs);
        for (auto itAddr = _registedClAddrs->begin(); itAddr != _registedClAddrs->end(); /*intenedTobeEmpy*/) {
            auto ec = sendMessageToClient(serviceStatusMsg, *itAddr);
            if((ec != DataTransmissionErrorCode::Success) && (ec != DataTransmissionErrorCode::ReceiverBusy))
            {
                //Client has been off, then don't keep their contact anymore
                itAddr = _registedClAddrs->erase(itAddr);
            }
            else
            {
                ++itAddr;
            }
        }
    }
}

bool IPCServerBase::onIncomingMessage(const CSMessagePtr &csMsg)
{
    mafInfo(csMsg);
    if(csMsg->operationCode() == OpCode::RegisterServiceStatus)
    {
        _registedClAddrs.atomic()->insert(csMsg->sourceAddress());
        {
            std::lock_guard lock(_providers);
            for(auto& provider : *_providers)
            {
                notifyServiceStatusToClient(csMsg->sourceAddress(), provider->serviceID(), Availability::Unavailable, Availability::Available);
            }
        }
        return true;
    }
    else
    {
        return ServerBase::onIncomingMessage(csMsg);
    }
}

void IPCServerBase::notifyServiceStatusToClient(const Address &clAddr, ServiceID sid, Availability oldStatus, Availability newStatus)
{
    if(oldStatus != newStatus)
    {
        mafInfo("Update service id " << sid << " status to client at address: " << clAddr.dump());
        auto serviceStatusMsg = createCSMessage<IPCMessage>(sid, newStatus == Availability::Available ? OpID_ServiceAvailable : OpID_ServiceUnavailable, OpCode::ServiceStatusUpdate);
        auto ec = sendMessageToClient(serviceStatusMsg, clAddr);
        if((ec != DataTransmissionErrorCode::Success) && (ec != DataTransmissionErrorCode::ReceiverBusy))
        {
            //Don't need to remove client if failed?
        }
    }
}


} // ipc
} // messaging
} // maf
