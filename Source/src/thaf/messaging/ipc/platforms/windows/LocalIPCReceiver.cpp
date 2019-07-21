#include "thaf/messaging/ipc/prv/platforms/windows/LocalIPCReceiver.h"
//#include "thaf/messaging/ipc/prv/platforms/windows/NamedPipeReceiver.h"
#include "thaf/messaging/ipc/prv/platforms/windows/OverlappedPipeReceiver.h"


namespace thaf {
namespace messaging {
namespace ipc {

LocalIPCReceiver::LocalIPCReceiver()
{
    _impl = std::make_unique<OverlappedPipeReceiver>();
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
} // thaf