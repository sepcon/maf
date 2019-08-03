
#include "thaf/messaging/client-server/ipc/internal/LocalIPCSender.h"
#if defined(_WIN32) || defined(_WIN64)
#   include "./platforms/windows/LocalIPCSenderImpl.cpp"
#elif defined(LINUX)
#   include "./platforms/linux/LocalIPCSenderImpl.cpp"
#endif

namespace thaf {
namespace messaging {
namespace ipc {

LocalIPCSender::LocalIPCSender()
{
    _pImpl = std::make_unique<LocalIPCSenderImpl>();

}

LocalIPCSender::~LocalIPCSender()
{
}

void LocalIPCSender::initConnection(const Address& addr)
{
    _pImpl->initConnection(addr);
}

DataTransmissionErrorCode LocalIPCSender::send(const thaf::srz::ByteArray& ba, const Address &destination)
{
    return _pImpl->send(ba, destination);
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




