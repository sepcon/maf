#include "thaf/messaging/ipc/IPCClientBase.h"
#include "thaf/messaging/ipc/BytesCommunicator.h"


namespace thaf {
namespace messaging {
namespace ipc {


IPCClientBase::IPCClientBase() :
    BytesCommunicator(this)
{
}

void IPCClientBase::init(IPCType type, const Address &serverAddress, long long serverStatusCheckPeriodMS)
{
    BytesCommunicator::init(type, serverAddress, /*isclient = */true);
    monitorServerStatus(serverStatusCheckPeriodMS);
}

void IPCClientBase::deinit()
{
    ClientBase::deinit();
    BytesCommunicator::deinit();
    _stop.store(true, std::memory_order_release);
    if(_serverMonitorThread.joinable())
    {
        _serverMonitorThread.join();
    }
    auto lock(_requesters.pa_lock());
    _requesters->clear();
}

IPCClientBase::~IPCClientBase()
{
    deinit();
}

DataTransmissionErrorCode IPCClientBase::sendMessageToServer(const CSMessagePtr &msg)
{
    msg->setSourceAddress(_pReceiver->address());
    return BytesCommunicator::send(std::static_pointer_cast<IPCMessage>(msg));
}

void IPCClientBase::onServerStatusChanged(Availability oldStatus, Availability newStatus)
{
    if(newStatus == Availability::Available)
    {
        auto registeredMsg = messaging::createCSMessage<IPCMessage>(ServiceIDInvalid, OpIDInvalid, OpCode::ServiceStatusUpdate);
        sendMessageToServer(registeredMsg);
    }
    ClientBase::onServerStatusChanged(oldStatus, newStatus);
}

void IPCClientBase::monitorServerStatus(long long serverStatusCheckPeriodMS)
{
    _stop.store(false, std::memory_order_release);
    _serverMonitorThread = std::thread {
        [this, serverStatusCheckPeriodMS]{
            Availability oldStatus = Availability::Unavailable;
            while(!_stop.load(std::memory_order_acquire))
            {
                auto newStatus = _pSender->checkReceiverStatus();
                if(oldStatus != newStatus)
                {
                    onServerStatusChanged(oldStatus, newStatus);
                    oldStatus = newStatus;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(serverStatusCheckPeriodMS));
            }
        }
    };
}

} // ipc
} // messaging
} // thaf
