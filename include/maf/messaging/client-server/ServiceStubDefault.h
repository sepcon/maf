#pragma once

#include <maf/messaging/client-server/ServiceStubHandlerInterface.h>
#include <maf/messaging/client-server/ServerFactory.h>
#include <maf/messaging/client-server/ServiceProvider.h>
#include <maf/threading/Lockable.h>
#include <map>

namespace maf {
namespace messaging {

class ServiceStubDefault : public ServiceProvider, public ServiceStubHandlerInterface
{
    using _MyBase                   = ServiceProvider;
protected:
    using PropertyPtr               = CSMsgContentBasePtr;
    using PropertyMap               = threading::Lockable<std::map<OpID, PropertyPtr>>;
    using RequestPtr                = std::shared_ptr<RequestInterface>;
    using RequestHandlerFunction    = std::function<void(const RequestPtr&)>;
    using RequestHandlerMap         = threading::Lockable<std::map<OpID, RequestHandlerFunction>>;

    PropertyMap                     _propertyMap;
    RequestHandlerMap               _requestHandlerMap;

    // Derived class must override this method
    // void onClientRequest(const std::shared_ptr<RequestInterface>& request) override = 0;
    void onClientRequest(const std::shared_ptr<RequestInterface> &request) override;
    void updateLatestStatus(const RequestPtr& request);
    void handleStatusGetRequest(const RequestPtr &request);
    bool handleRequest(const RequestPtr& request);

public:
    ServiceStubDefault(ServiceID sid, std::weak_ptr<ServerInterface> server);
    ActionCallStatus setStatus(OpID propertyID, const CSMsgContentBasePtr& newStatus) override;
    CSMsgContentBasePtr getStatus(OpID propertyID);
    bool setRequestHandler(OpID opID, RequestHandlerFunction handlerFunction);
};

} // messaging
} // maf
