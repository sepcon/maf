#include "thaf/Messaging/IPC/IPCRequestTracker.h"
#include "thaf/Messaging/IPC/IPCServiceStub.h"


namespace thaf {
namespace messaging {
namespace ipc {


IPCRequestTracker::IPCRequestTracker(std::shared_ptr<IPCMessage> ipcMsg, IPCServiceStub *svStub) :
    _ipcMsg(std::move(ipcMsg)), _svStub(std::move(svStub)), _valid(true)
{
    assert(_svStub);
    assert(_ipcMsg);
}

std::shared_ptr<IPCRequestTracker> IPCRequestTracker::create(std::shared_ptr<IPCMessage> ipcMsg, IPCServiceStub *svStub)
{
    std::shared_ptr<IPCRequestTracker> clp(new IPCRequestTracker(std::move(ipcMsg), svStub));
    return clp;
}

OpCode IPCRequestTracker::getOpCode() const
{
    if(_ipcMsg->get_operation_code() == OpCode::RequestSync)
    {
        return OpCode::Request; // Currently there's no need to diffrentiate between sync/async request
    }
    else
    {
        return _ipcMsg->get_operation_code();
    }
}

OpID IPCRequestTracker::getOpID() const
{
    return _ipcMsg->get_operation_id();
}

bool IPCRequestTracker::valid() const
{
    return _valid.load(std::memory_order_acquire);
}

bool IPCRequestTracker::reply(const std::shared_ptr<IPCDataCarrier>& data)
{
    if(valid())
    {
        if(data->getID() == _ipcMsg->get_operation_id())
        {
            _ipcMsg->set_payload(data->toBytes());
            return _svStub->replyToRequest(_ipcMsg);
        }
        else
        {
            thafErr("Mismatched of OperationId between response data id[" << data->getID() << "] and request data id[" << _ipcMsg->get_operation_id());
            return false;
        }
    }
    else
    {
        thafErr("IPCReplyHelper is no longer valid, might be the operation id [" << _ipcMsg->get_operation_id() << "]");
        return false;
    }
}

void IPCRequestTracker::invalidate()
{
    _valid.store(false, std::memory_order_release);
}


} // ipc
} // messaging
} // thaf
