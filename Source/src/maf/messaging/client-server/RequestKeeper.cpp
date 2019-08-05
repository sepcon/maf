#include "maf/messaging/client-server/RequestKeeper.h"
#include "maf/messaging/client-server/ServiceStubBase.h"
#include "maf/utils/debugging/Debug.h"


namespace maf {
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

bool RequestKeeperBase::respond(const CSMsgContentPtr& answer, RequestResultStatus status)
{
    if(valid()) //BUG: must use mutex to prevent _svStub from being destroyed after checking valid
    {
        if(answer->operationID() == _csMsg->operationID())
        {
            _csMsg->setContent(answer);
            return _svStub->replyToRequest(_csMsg, status == RequestResultStatus::Complete);
        }
        else
        {
            mafErr("Mismatched of OperationId between response data id[" << answer->operationID() << "] and request data id[" << _csMsg->operationID());
            return false;
        }
    }
    else
    {
        mafErr("IPCReplyHelper is no longer valid, might be the operation id [" << _csMsg->operationID() << "]");
        return false;
    }
}

void RequestKeeperBase::update(const CSMsgContentPtr &answer)
{
    respond(answer, RequestResultStatus::Incomplete);
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
} // maf
