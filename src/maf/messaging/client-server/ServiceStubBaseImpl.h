#pragma once
#include <maf/messaging/client-server/CSStatus.h>
#include <maf/messaging/client-server/ServerInterface.h>
#include <maf/messaging/client-server/RequestKeeper.h>
#include <maf/utils/cppextension/Lockable.h>
#include <maf/utils/debugging/Debug.h>
#include <map>
#include <set>
#include <list>

namespace maf {
namespace messaging {
class ServiceStubHandlerInterface;

class ServiceStubBaseImpl
{
    using RequestKeeperPtr = std::shared_ptr<RequestKeeperBase>;
    using RequestKeeperMap = nstl::Lockable<std::map<OpID, std::list<RequestKeeperPtr>>>;
    using Address2OpIDsMap = nstl::Lockable<std::map<Address, std::set<OpID>>>;
    friend class RequestKeeperBase;
public:
    ServiceStubBaseImpl(ServerInterface* server, ServiceStubHandlerInterface* stubHandler = nullptr);
    ~ServiceStubBaseImpl();

    void setStubHandler(ServiceStubHandlerInterface *stubHandler);
    bool replyToRequest(const CSMessagePtr &csMsg, bool hasDone = true);
    bool sendStatusUpdate(const CSMessagePtr& csMsg);

    bool feedbackToClient(const CSMessagePtr &csMsg, bool hasDone = true);
    bool onIncomingMessage(const CSMessagePtr& msg);
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
    RequestKeeperMap _requestKeepersMap;
    ServerInterface* _server;
    std::atomic_bool _stopped;
};

}
}
