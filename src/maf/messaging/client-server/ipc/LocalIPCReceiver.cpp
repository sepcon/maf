#include <maf/utils/debugging/Debug.h>
#include <internal/LocalIPCReceiverImpl.h>
#include "LocalIPCReceiver.h"

namespace maf {
namespace messaging {
namespace ipc {

LocalIPCReceiver::LocalIPCReceiver()
{
    _impl = std::make_unique<LocalIPCReceiverImpl>();
    _impl->registerObserver(this);
}

LocalIPCReceiver::~LocalIPCReceiver()
{
}

bool LocalIPCReceiver::initConnection(Address address, bool isClientMode)
{
    return _impl->initConnection(address, isClientMode);
}

bool LocalIPCReceiver::startListening()
{
    return _impl->startListening();
}

bool LocalIPCReceiver::stopListening()
{
    return _impl->stopListening();
}

bool LocalIPCReceiver::listening() const
{
    return _impl->listening();
}

const Address &LocalIPCReceiver::address() const
{
    return _impl->address();
}

void LocalIPCReceiver::onBytesCome(const std::shared_ptr<srz::ByteArray> &bytes)
{
    notifyObervers(bytes);
}



} // ipc
} // messaging
} // maf
