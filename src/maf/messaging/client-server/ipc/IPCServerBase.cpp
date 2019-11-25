#include <maf/logging/Logger.h>
#include <maf/messaging/client-server/ipc/IPCServerBase.h>
#include <maf/messaging/client-server/ipc/BytesCommunicator.h>
#include <maf/messaging/client-server/ServiceProviderInterface.h>

namespace maf { using logging::Logger;
namespace messaging {
namespace ipc {

IPCServerBase::IPCServerBase(IPCType ipctype) :
    _communicator(new BytesCommunicator(ipctype, this, /*isclient = */false))
{
}

IPCServerBase::~IPCServerBase()
{
    delete _communicator;
}

bool IPCServerBase::init(const Address &serverAddress)
{
    return _communicator->init(serverAddress);
}

bool IPCServerBase::deinit()
{
    return _communicator->deinit() && ServerBase::deinit();
}

ActionCallStatus IPCServerBase::sendMessageToClient(const CSMessagePtr &msg, const Address &addr)
{
    return _communicator->send(std::static_pointer_cast<IPCMessage>(msg), addr);
}

void IPCServerBase::notifyServiceStatusToClient(const ServiceID& sid, Availability oldStatus, Availability newStatus)
{
    if(oldStatus != newStatus)
    {
        auto serviceStatusMsg = createCSMessage<IPCMessage>(sid, newStatus == Availability::Available ? OpID_ServiceAvailable : OpID_ServiceUnavailable, OpCode::ServiceStatusUpdate);
        std::lock_guard lock(_registedClAddrs);
        for (auto itAddr = _registedClAddrs->begin(); itAddr != _registedClAddrs->end(); /*intenedTobeEmpy*/) {
            auto ec = sendMessageToClient(serviceStatusMsg, *itAddr);
            if((ec == ActionCallStatus::ReceiverUnavailable) || (ec == ActionCallStatus::FailedUnknown))
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
    switch (csMsg->operationCode()) {
    case OpCode::RegisterServiceStatus:
        _registedClAddrs.atomic()->insert(csMsg->sourceAddress());
        {
            std::lock_guard lock(_providers);
            for(auto& [sid, provider] : *_providers)
            {
                notifyServiceStatusToClient(csMsg->sourceAddress(), provider->serviceID(), Availability::Unavailable, Availability::Available);
            }
        }
        return true;

    case OpCode::UnregisterServiceStatus:
        if(csMsg->serviceID() == ServiceIDInvalid)
        {
            _registedClAddrs.atomic()->erase(csMsg->sourceAddress());
            std::lock_guard lock(_providers);
            for(auto& [sid, provider] : *_providers)
            {
                csMsg->setServiceID(sid);
                provider->onIncomingMessage(csMsg);
            }
            return true;
        }
        else
        {
            break;
        }
    default: break;
    }

    return ServerBase::onIncomingMessage(csMsg);
}

void IPCServerBase::notifyServiceStatusToClient(const Address &clAddr, const ServiceID& sid, Availability oldStatus, Availability newStatus)
{
    if(oldStatus != newStatus)
    {
        Logger::info("Update service id " ,  sid ,  " status to client at address: " ,  clAddr.dump());
        auto serviceStatusMsg = createCSMessage<IPCMessage>(sid, newStatus == Availability::Available ? OpID_ServiceAvailable : OpID_ServiceUnavailable, OpCode::ServiceStatusUpdate);
        auto ec = sendMessageToClient(serviceStatusMsg, clAddr);
        if((ec != ActionCallStatus::Success) && (ec != ActionCallStatus::ReceiverBusy))
        {
            //Don't need to remove client if failed?
        }
    }
}


} // ipc
} // messaging
} // maf
