#include <maf/logging/Logger.h>
#include <maf/messaging/client-server/ipc/IPCClientBase.h>
#include <maf/messaging/client-server/ipc/BytesCommunicator.h>


namespace maf { using logging::Logger;
namespace messaging {
namespace ipc {


IPCClientBase::IPCClientBase(IPCType type) :
    BytesCommunicator(type, this, /*isclient = */true)
{
}

bool IPCClientBase::init(
    const Address &serverAddress,
    long long sersverMonitoringCycleMS
    )
{
    if(BytesCommunicator::init(serverAddress))
    {
        monitorServerStatus(sersverMonitoringCycleMS);
        return true;
    }
    else
    {
        return false;
    }
}

bool IPCClientBase::deinit()
{
    _stop.store(true, std::memory_order_release);
    ClientBase::deinit();
    BytesCommunicator::deinit();
    if(_serverMonitorThread.joinable())
    {
        _serverMonitorThread.join();
    }
    return true;
}

IPCClientBase::~IPCClientBase()
{
    deinit();
}

ActionCallStatus IPCClientBase::sendMessageToServer(const CSMessagePtr &msg)
{
    msg->setSourceAddress(_pReceiver->address());
    return BytesCommunicator::send(std::static_pointer_cast<IPCMessage>(msg));
}

void IPCClientBase::onServerStatusChanged(
    Availability oldStatus,
    Availability newStatus
    )
{
    ClientBase::onServerStatusChanged(oldStatus, newStatus);
    if(newStatus == Availability::Available)
    {
        auto registeredMsg = messaging::createCSMessage<IPCMessage>(
            ServiceIDInvalid,
            OpIDInvalid,
            OpCode::RegisterServiceStatus
            );
        if (sendMessageToServer(registeredMsg) == ActionCallStatus::Success)
        {
            Logger::info("Send service status change register to server successfully!");
        }
        else
        {
            Logger::info("Could not send service status register request to server");
        }
    }
}

void IPCClientBase::monitorServerStatus(long long sersverMonitoringCycleMS)
{
    _stop.store(false, std::memory_order_release);
    _serverMonitorThread = std::thread {
        [this, sersverMonitoringCycleMS] {
            Availability oldStatus = Availability::Unavailable;
            while(!_stop.load(std::memory_order_acquire))
            {
                auto newStatus = _pSender->checkReceiverStatus();
                if(oldStatus != newStatus)
                {
                    onServerStatusChanged(oldStatus, newStatus);
                    oldStatus = newStatus;
                }
                std::this_thread::sleep_for(
                    std::chrono::milliseconds(sersverMonitoringCycleMS)
                    );
            }
        }
    };
}

} // ipc
} // messaging
} // maf
