#pragma once

#include "ServiceStubInterface.h"
#include "ServerInterface.h"
#include "maf/utils/cppextension/SyncObject.h"

namespace maf {
namespace messaging {


class ServiceStubBase : public ServiceStubInterface
{
    using RequestKeeperPtr = std::shared_ptr<RequestKeeperBase>;
    using OpID2RequestKeeperMap = nstl::SyncObject<std::map<OpID, std::list<RequestKeeperPtr>>>;
    using Address2OpIDsMap = nstl::SyncObject<std::map<Address, std::set<OpID>>>;
    friend class RequestKeeperBase;
public:
    ServiceStubBase(ServiceID sid, ServerInterface* server, ServiceStubHandlerInterface* stubHandler = nullptr);
    ~ServiceStubBase() override;

    void setStubHandler(ServiceStubHandlerInterface *stubHandler) override;
    bool replyToRequest(const CSMessagePtr &csMsg, bool hasDone = true) override;
    bool sendStatusUpdate(const CSMessagePtr& csMsg) override;

protected:
    bool feedbackToClient(const CSMessagePtr &csMsg, bool hasDone = true);
    bool onIncomingMessage(const CSMessagePtr& msg) override;
    void onStatusChangeRegister(const CSMessagePtr& msg);
    void onStatusChangeUnregister(const CSMessagePtr& msg);
    void forwardToStubHandler(const RequestKeeperPtr& requestKeeper);
    void forwardToStubHandler(RequestKeeperBase::AbortCallback callback);

    RequestKeeperPtr saveRequestInfo(const CSMessagePtr& msg);
    RequestKeeperPtr pickOutRequestInfo(const CSMessagePtr &msgContent, bool remove = true);
    void invalidateAndRemoveAllRequestKeepers();

    void saveRegisterInfo(const CSMessagePtr& msg);
    void removeRegisterInfo(const CSMessagePtr& msg);
    void removeAllRegisterInfo();
    void removeRegistersOfAddress(const Address& addr);

    void onAbortActionRequest(const CSMessagePtr& msg);
    void onClientRequest(const CSMessagePtr& msg);


    ServiceStubHandlerInterface* _stubHandler;
    Address2OpIDsMap _regEntriesMap;
    OpID2RequestKeeperMap _requestKeepersMap;
    ServerInterface* _server;
    std::atomic_bool _stopped;
};

}// messaging
}// maf
