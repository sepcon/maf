#pragma once

#include "interfaces/ServiceStubInterface.h"
#include "interfaces/ServerInterface.h"
#include "thaf/utils/cppextension/SyncObject.h"

namespace thaf {
namespace messaging {


class ServiceStubBase : public ServiceStubInterface
{
    using RequestKeeperPtr = std::shared_ptr<RequestKeeperBase>;
    using OpID2RequestKeeperMap = stl::SyncObjectM<std::map<OpID, std::list<RequestKeeperPtr>>>;
    using Address2OpIDsMap = stl::SyncObjectM<std::map<Address, std::set<OpID>>>;

public:
    ServiceStubBase(ServiceID sid, ServerInterface* server, ServiceStubHandlerInterface* stubHandler = nullptr);
    ~ServiceStubBase() override;

    void setStubHandler(ServiceStubHandlerInterface *stubHandler) override;
    bool replyToRequest(const CSMessagePtr &msgContent) override;
    bool sendStatusUpdate(const CSMessagePtr& csMsg) override;

protected:
    bool onIncomingMessage(const CSMessagePtr& msg) override;
    void onStatusChangeRegister(const CSMessagePtr& msg);
    void onStatusChangeUnregister(const CSMessagePtr& msg);
    void forwardToStubHandler(const RequestKeeperPtr& requestKeeper);

    RequestKeeperPtr saveRequestInfo(const CSMessagePtr& msg);
    RequestKeeperPtr removeRequestInfo(const CSMessagePtr &msgContent);
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
    util::IDManager _idMgr;
};

}// messaging
}// thaf
