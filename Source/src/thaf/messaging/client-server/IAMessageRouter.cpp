#include "thaf/messaging/client-server/IAMessageRouter.h"
#include "thaf/messaging/client-server/ServiceRequesterInterface.h"
#include "thaf/messaging/client-server/ServiceProviderInterface.h"
#include "thaf/utils/debugging/Debug.h"

namespace thaf {
namespace messaging {


void IAMessageRouter::init()
{
    ClientBase::init();
    ServerBase::init();
}

void IAMessageRouter::deinit()
{
    ClientBase::deinit();
    ClientBase::deinit();
}

bool IAMessageRouter::registerServiceRequester(const std::shared_ptr<ServiceRequesterInterface> &requester)
{
    if(ClientBase::registerServiceRequester(requester))
    {
        if(ServerBase::hasServiceProvider(requester->serviceID()))
        {
            thafMsg("\n------------IAMessageRouter::registerServiceRequester--------------------\n");
            requester->onServiceStatusChanged(requester->serviceID(), Availability::Unavailable, Availability::Available);
        }
        return true;
    }
    else
    {
        return false;
    }
}

DataTransmissionErrorCode IAMessageRouter::sendMessageToClient(const CSMessagePtr &msg, const Address &/*addr*/)
{
    if(ClientBase::onIncomingMessage(msg))
    {
        return DataTransmissionErrorCode::Success;
    }
    else {
        return DataTransmissionErrorCode::ReceiverUnavailable;
    }
}

DataTransmissionErrorCode IAMessageRouter::sendMessageToServer(const CSMessagePtr &msg)
{
    if(ServerBase::onIncomingMessage(msg))
    {
        return DataTransmissionErrorCode::Success;
    }
    else
    {
        return DataTransmissionErrorCode::ReceiverUnavailable;
    }
}

void IAMessageRouter::notifyServiceStatusToClient(ServiceID sid, Availability oldStatus, Availability newStatus)
{
    ClientBase::onServiceStatusChanged(sid, oldStatus, newStatus);
}

}
}
