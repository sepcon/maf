#include "thaf/messaging/client-server/ipc/IPCReceiver.h"


namespace thaf {
namespace messaging {
namespace ipc {

void IPCReceiver::registerObserver(BytesComeObserver* observer)
{
    if(observer)
    {
        _observers.push_back(observer);
    }
}

void IPCReceiver::unregisterObserver(BytesComeObserver *observer)
{
    auto it = _observers.begin();
    while(it != _observers.end())
    {
        if(*it == observer)
        {
            it = _observers.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void IPCReceiver::notifyObervers(const std::shared_ptr<srz::ByteArray>& bytes)
{
    for (auto& observer : _observers)
    {
        observer->onBytesCome(bytes);
    }
}


}
}
}
