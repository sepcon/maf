#include "thaf/Messaging/IPC/Prv/Platforms/Windows/LocalIPCSender.h"
//#include "thaf/Messaging/IPC/Prv/Platforms/Windows/NamedPipeSender.h"
#include "thaf/Messaging/IPC/Prv/Platforms/Windows/OverlappedPipeSender.h"


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

ConnectionErrorCode LocalIPCSender::send(const thaf::srz::ByteArray& ba)
{
    return _pImpl->send(ba);
}

const Address &LocalIPCSender::receiverAddress() const
{
    return _pImpl->receiverAddress();
}

ConnectionStatus LocalIPCSender::checkServerStatus() const
{
    return _pImpl->checkServerStatus();
}




} // ipc
} // messaging
} // thaf




