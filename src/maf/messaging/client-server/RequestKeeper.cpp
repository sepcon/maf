#include <maf/messaging/client-server/RequestKeeper.h>
#include <maf/messaging/client-server/ServiceStubBase.h>
#include <maf/utils/debugging/Debug.h>
#include "ServiceStubBaseImpl.h"


namespace maf {
namespace messaging {


RequestKeeperBase::RequestKeeperBase(std::shared_ptr<CSMessage> csMsg, ServiceStubBaseImpl *svStub) :
    _csMsg(std::move(csMsg)), _svStub(std::move(svStub)), _valid(true)
{
    assert(_svStub);
    assert(_csMsg);
}

RequestKeeperBase::AbortCallback RequestKeeperBase::getAbortCallback()
{
    std::lock_guard<std::mutex> lock(_mutex);
    return _abortCallback;
}

std::shared_ptr<RequestKeeperBase> RequestKeeperBase::create(std::shared_ptr<CSMessage> csMsg, ServiceStubBaseImpl *svStub)
{
    std::shared_ptr<RequestKeeperBase> instance(new RequestKeeperBase(std::move(csMsg), svStub));
    return instance;
}

OpCode RequestKeeperBase::getOperationCode() const
{
    std::lock_guard<std::mutex> lock(_mutex);
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
    std::lock_guard<std::mutex> lock(_mutex);
    return _csMsg->operationID();
}

bool RequestKeeperBase::valid() const
{
    std::lock_guard<std::mutex> lock(_mutex);
    return _valid;
}

bool RequestKeeperBase::respond(const CSMsgContentPtr& answer)
{
    constexpr bool DONE = true;
    std::lock_guard<std::mutex> lock(_mutex);
    // remove keeper from tracked list of stub
    if(_svStub) { _svStub->pickOutRequestInfo(_csMsg); }

    return sendMsgToClient(answer, DONE);
}

bool RequestKeeperBase::update(const CSMsgContentPtr &answer)
{
    constexpr bool NOT_DONE = false;
    std::lock_guard<std::mutex> lock(_mutex);
    return sendMsgToClient(answer, NOT_DONE);
}

CSMsgContentPtr RequestKeeperBase::getRequestContent()
{
    std::lock_guard<std::mutex> lock(_mutex);
    return _csMsg->content();
}

void RequestKeeperBase::abortedBy(AbortCallback abortCallback)
{
    std::lock_guard<std::mutex> lock(_mutex);
    if(!_valid)
    {
        abortCallback();
    }
    else
    {
        _abortCallback = std::move(abortCallback);
    }
}


bool RequestKeeperBase::invalidateIfValid()
{
    std::lock_guard<std::mutex> lock(_mutex);
    if(_valid)
    {
        _valid = false;
        _svStub = nullptr;
        return true;
    }
    else
    {
        return false;
    }
}

bool RequestKeeperBase::sendMsgToClient(const CSMsgContentPtr &answer, bool done)
{
    if(_valid)
    {
        if(answer->operationID() == _csMsg->operationID())
        {
            _csMsg->setContent(answer);
            return _svStub->feedbackToClient(_csMsg, done);
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


} // messaging
} // maf
