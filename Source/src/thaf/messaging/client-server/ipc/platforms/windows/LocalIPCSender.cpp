#include "thaf/messaging/client-server/ipc/internal/platforms/windows/LocalIPCSender.h"
//#include "thaf/messaging/client-server/ipc/internal/platforms/windows/NamedPipeSender.h"
#include "thaf/messaging/client-server/ipc/internal/platforms/windows/OverlappedPipeSender.h"


namespace thaf {
namespace messaging {
namespace ipc {

LocalIPCSender::LocalIPCSender()
{
    _pImpl = std::make_unique<OverlappedPipeSender>();

}

LocalIPCSender::~LocalIPCSender()
{
}

void LocalIPCSender::initConnection(const Address& addr)
{
    _pImpl->initConnection(addr);
}

DataTransmissionErrorCode LocalIPCSender::send(const thaf::srz::ByteArray& ba)
{
    return _pImpl->send(ba);
}

const Address &LocalIPCSender::receiverAddress() const
{
    return _pImpl->receiverAddress();
}

Availability LocalIPCSender::checkReceiverStatus() const
{
    return _pImpl->checkReceiverStatus();
}




} // ipc
} // messaging
} // thaf




