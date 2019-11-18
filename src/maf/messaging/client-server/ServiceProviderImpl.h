#pragma once
#include <maf/messaging/client-server/internal/CSShared.h>
#include <maf/messaging/client-server/CSStatus.h>
#include <maf/messaging/client-server/ServiceProviderInterface.h>
#include <maf/threading/Lockable.h>
#include <map>
#include <set>
#include <list>

namespace maf {
namespace messaging {

class ServerInterface;
class ServiceProvider;
class Request;
class ServiceStubHandlerInterface;

struct ServiceProviderImpl
{
    using RequestPtr                = std::shared_ptr<Request>;
    using RequestMap                = threading::Lockable<std::map<OpID, std::list<RequestPtr>>>;
    using Address2OpIDsMap          = threading::Lockable<std::map<Address, std::set<OpID>>>;

    Address2OpIDsMap                            _regEntriesMap;
    RequestMap                                  _requestsMap;
    std::weak_ptr<ServerInterface>              _server;
    ServiceProvider*                            _holder;
    ServiceStubHandlerInterface*                _stubHandler        = nullptr;
    std::atomic_bool                            _stopped;

    ServiceProviderImpl(
        ServiceProvider* holder,
        std::weak_ptr<ServerInterface> server,
        ServiceStubHandlerInterface* stubHandler = nullptr
        );

    ~ServiceProviderImpl();

    void setStubHandler(ServiceStubHandlerInterface *stubHandler);
    ActionCallStatus respondToRequest(const CSMessagePtr &csMsg);
    ActionCallStatus setStatus(
        OpID propertyID,
        const CSMsgContentBasePtr& property
        );

    void startServing();
    void stopServing();

    bool onIncomingMessage(const CSMessagePtr& msg);

    ActionCallStatus sendMessage(const CSMessagePtr &csMsg, const Address &toAddr);
    ActionCallStatus sendBackMessageToClient(const CSMessagePtr &csMsg);
    void onStatusChangeRegister(const CSMessagePtr& msg);
    void onStatusChangeUnregister(const CSMessagePtr& msg);
    void forwardToStubHandler(const RequestPtr& request);
    void forwardToStubHandler(RequestAbortedCallback callback);

    RequestPtr saveRequestInfo(const CSMessagePtr& msg);
    RequestPtr pickOutRequestInfo(const CSMessagePtr &msgContent);

    void invalidateAndRemoveAllRequests();


    void saveRegisterInfo(const CSMessagePtr& msg);
    void removeRegisterInfo(const CSMessagePtr& msg);
    void removeAllRegisterInfo();
    void removeRegistersOfAddress(const Address& addr);

    void onAbortActionRequest(const CSMessagePtr& msg);
    void onClientRequest(const CSMessagePtr& msg);
    void onClientGoesOff(const CSMessagePtr& msg);

};

}
}
