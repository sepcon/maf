#pragma once

#include "thaf/Messaging/IPC/IPCInfo.h"
#include "ClientServerContract.h"

namespace thaf {
namespace messaging {
namespace ipc {

struct IPCMessage;
class IPCServiceStub;

class IPCRequestTracker
{
public:
    OpCode getOpCode() const;
    OpID getOpID() const;
    bool valid() const;
    bool reply(const std::shared_ptr<IPCDataCarrier>& data);
    template<typename DataCarrier, std::enable_if_t<std::is_base_of_v<IPCDataCarrier, DataCarrier>, bool> = true>
    std::shared_ptr<DataCarrier> getDataCarrier() const noexcept;

private:
    friend class IPCServiceStub;
    static std::shared_ptr<IPCRequestTracker> create(std::shared_ptr<IPCMessage> ipcMsg, messaging::ipc::IPCServiceStub* svStub);
    IPCRequestTracker(std::shared_ptr<IPCMessage> ipcMsg, messaging::ipc::IPCServiceStub* svStub);
    void invalidate();

    std::shared_ptr<messaging::ipc::IPCMessage> _ipcMsg;
    messaging::ipc::IPCServiceStub* _svStub;
    std::atomic_bool _valid;
};

template<typename DataCarrier, std::enable_if_t<std::is_base_of_v<IPCDataCarrier, DataCarrier>, bool> >
std::shared_ptr<DataCarrier> IPCRequestTracker::getDataCarrier() const noexcept
{
    assert(std::is_default_constructible_v<DataCarrier>);
    auto data = std::make_shared<DataCarrier>();
    IPCDataCarrier& ipcData = static_cast<IPCDataCarrier&>(*data);
    if(ipcData.getID() != _ipcMsg->get_operation_id())
    {
        data.reset();
    }
    else
    {
        try
        {
            ipcData.fromBytes(_ipcMsg->get_payload());
        }
        catch(const std::exception& e)
        {
            thafErr("Could not decode data with id: [" << ipcData.getID() << "] came from address: " << _ipcMsg->get_sender_addr().dump());
            thafErr("Exception: <<" << e.what() << ">>");
            thafErr(_ipcMsg->get_payload());
        }
    }
    return data;
}

} // ipc
} // messaging
} // thaf
