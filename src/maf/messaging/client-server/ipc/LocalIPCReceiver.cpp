#include "platforms/LocalIPCReceiver.h"

#if defined(_WIN32) || defined(_WIN64)
    #include "./platforms/windows/LocalIPCReceiverImpl.cpp"
#elif defined(LINUX)
    #include "./platforms/linux/LocalIPCReceiverImpl.cpp"
#else
namespace maf {
namespace messaging {
namespace ipc {

class LocalIPCReceiverImpl : public LocalIPCReceiver
{
public:
    virtual bool initConnection(Address, bool = false)  { return false; }
    virtual bool startListening()  { return false; }
    virtual bool stopListening()  { return false; }
    virtual bool listening() const  { return false; }
    virtual const Address& address() const  { return Address::INVALID_ADDRESS; }
};
}
}
}
#endif

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
