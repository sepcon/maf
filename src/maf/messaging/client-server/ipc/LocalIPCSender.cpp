
#include "maf/messaging/client-server/ipc/internal/LocalIPCSender.h"
#if defined(_WIN32) || defined(_WIN64)
#   include "./platforms/windows/LocalIPCSenderImpl.cpp"
#elif defined(LINUX)
#   include "./platforms/linux/LocalIPCSenderImpl.cpp"
#else
namespace maf {
namespace messaging {
namespace ipc {

class LocalIPCSenderImpl : public IPCSender
{
public:
    virtual void initConnection(const Address&) {}
    virtual DataTransmissionErrorCode send(const srz::ByteArray&, const Address& = Address::INVALID_ADDRESS)
    { return {}; }
    virtual Availability checkReceiverStatus() const { return {}; }
    virtual const Address& receiverAddress() const { return Address::INVALID_ADDRESS; }
};

}}}
#endif

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

void LocalIPCSender::initConnection(const Address& addr)
{
    _pImpl->initConnection(addr);
}

DataTransmissionErrorCode LocalIPCSender::send(const maf::srz::ByteArray& ba, const Address &destination)
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




