#include <internal/LocalIPCSenderImpl.h>
#include "LocalIPCSender.h"


namespace maf {
namespace messaging {
namespace ipc {

LocalIPCSender::LocalIPCSender()
{
    _pImpl = std::make_unique<LocalIPCSenderImpl>();

}

LocalIPCSender::~LocalIPCSender()
{
}

bool LocalIPCSender::initConnection(const Address& addr)
{
    return _pImpl->initConnection(addr);
}

ActionCallStatus LocalIPCSender::send(const maf::srz::ByteArray& ba, const Address &destination)
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
} // maf




