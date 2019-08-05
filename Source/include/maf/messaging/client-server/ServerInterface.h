#pragma once

#include "CSMessage.h"
#include "CSStatus.h"
#include "CSMessageReceiver.h"

namespace maf {
namespace messaging {

class ServiceProviderInterface;
using IServiceProviderPtr = std::shared_ptr<ServiceProviderInterface>;

class ServerInterface : public CSMessageReceiver
{
public:
    virtual ~ServerInterface() = default;
    virtual DataTransmissionErrorCode sendMessageToClient(const CSMessagePtr& msg, const Address& addr) = 0;
    virtual bool registerServiceProvider(const IServiceProviderPtr& serviceProvider) = 0;
    virtual bool unregisterServiceProvider(const IServiceProviderPtr& serviceProvider) = 0;
    virtual bool unregisterServiceProvider(ServiceID sid) = 0;
    virtual bool hasServiceProvider(ServiceID sid) = 0;
    virtual IServiceProviderPtr getServiceProvider(ServiceID sid) = 0;
};

}
}
