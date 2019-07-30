#include "thaf/messaging/client-server/RequestKeeper.h"
#include "thaf/messaging/client-server/ServiceStubBase.h"
#include "thaf/utils/debugging/Debug.h"


namespace thaf {
namespace messaging {


RequestKeeperBase::RequestKeeperBase(std::shared_ptr<CSMessage> csMsg, ServiceStubBase *svStub) :
    _csMsg(std::move(csMsg)), _svStub(std::move(svStub)), _valid(true)
{
    assert(_svStub);
    assert(_csMsg);
}

std::shared_ptr<RequestKeeperBase> RequestKeeperBase::create(std::shared_ptr<CSMessage> csMsg, ServiceStubBase *svStub)
{
    std::shared_ptr<RequestKeeperBase> instance(new RequestKeeperBase(std::move(csMsg), svStub));
    return instance;
}

OpCode RequestKeeperBase::getOperationCode() const
{
    if(_csMsg->operationCode() == OpCode::RequestSync)
    {
        return OpCode::Request; // Currently there's no need to diffrentiate between sync/async request
    }
    else
    {
        return _csMsg->operationCode();
    }
}

OpID RequestKeeperBase::getOperationID() const
{
    return _csMsg->operationID();
}

bool RequestKeeperBase::valid() const
{
    return _valid.load(std::memory_order_acquire);
}

bool RequestKeeperBase::reply(const CSMsgContentPtr& answer)
{
    if(valid())
    {
        if(answer->operationID() == _csMsg->operationID())
        {
            _csMsg->setContent(answer);
            return _svStub->replyToRequest(_csMsg);
        }
        else
        {
            thafErr("Mismatched of OperationId between response data id[" << answer->operationID() << "] and request data id[" << _csMsg->operationID());
            return false;
        }
    }
    else
    {
        thafErr("IPCReplyHelper is no longer valid, might be the operation id [" << _csMsg->operationID() << "]");
        return false;
    }
}

CSMsgContentPtr RequestKeeperBase::getRequestContent()
{
    return _csMsg->content();
}

void RequestKeeperBase::invalidate()
{
    _valid.store(false, std::memory_order_release);
}


} // messaging
} // thaf
