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

struct ServiceProviderImpl
{
    template <typename ValueType>
    using OpIDMap                = threading::Lockable<std::map<OpID, ValueType>>;

    using RequestPtr             = std::shared_ptr<Request>;
    using PropertyPtr            = CSMsgContentBasePtr;
    using RequestMap             = OpIDMap<std::list<RequestPtr>>;
    using PropertyMap            = OpIDMap<PropertyPtr>;
    using RequestHandlerMap      = OpIDMap<RequestHandlerFunction>;
    using Address2OpIDsMap       = threading::Lockable<std::map<Address, std::set<OpID>>>;

    Address2OpIDsMap                _regEntriesMap;
    RequestMap                      _requestsMap;
    std::weak_ptr<ServerInterface>  _server;
    ServiceProvider*                _delegator;
    PropertyMap                     _propertyMap;
    RequestHandlerMap               _requestHandlerMap;

    ServiceProviderImpl(
        ServiceProvider* holder,
        std::weak_ptr<ServerInterface> server
        );

    ~ServiceProviderImpl();

    ActionCallStatus respondToRequest(const CSMessagePtr &csMsg);
    ActionCallStatus setStatus(
        OpID propertyID,
        const CSMsgContentBasePtr& property
        );

    CSMsgContentBasePtr getStatus(OpID propertyID);

    void startServing();
    void stopServing();

    bool onIncomingMessage(const CSMessagePtr& msg);

    ActionCallStatus sendMessage(const CSMessagePtr &csMsg, const Address &toAddr);
    ActionCallStatus sendBackMessageToClient(const CSMessagePtr &csMsg);
    void onStatusChangeRegister(const CSMessagePtr& msg);
    void onStatusChangeUnregister(const CSMessagePtr& msg);


    RequestPtr saveRequestInfo(const CSMessagePtr& msg);
    RequestPtr pickOutRequestInfo(const CSMessagePtr &msgContent);

    void invalidateAndRemoveAllRequests();


    void saveRegisterInfo(const CSMessagePtr& msg);
    void removeRegisterInfo(const CSMessagePtr& msg);
    void removeAllRegisterInfo();
    void removeRegistersOfAddress(const Address& addr);

    void onAbortActionRequest(const CSMessagePtr& msg);
    void onClientGoesOff(const CSMessagePtr& msg);

    void onActionRequest(const CSMessagePtr& msg);
    void updateLatestStatus(const CSMessagePtr& registerMsg);
    void onStatusGetRequest(const CSMessagePtr &getMsg);
    bool invokeRequestHandlerCallback(const RequestPtr& request);

    bool registerRequestHandler(OpID opID, RequestHandlerFunction handlerFunction);
    bool unregisterRequestHandler( OpID opID );

};

}
}
