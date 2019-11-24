#pragma once

#include "CSMessage.h"
#include "CSStatus.h"
#include "CSMessageReceiver.h"

namespace maf {
namespace messaging {

class ServiceProviderInterface;
using ServiceProviderInterfacePtr = std::shared_ptr<ServiceProviderInterface>;

class ServerInterface : public CSMessageReceiver
{
public:
    virtual ~ServerInterface() = default;
    virtual ActionCallStatus sendMessageToClient(const CSMessagePtr& msg, const Address& addr) = 0;
    virtual bool registerServiceProvider(const ServiceProviderInterfacePtr& serviceProvider) = 0;
    virtual bool unregisterServiceProvider(const ServiceProviderInterfacePtr& serviceProvider) = 0;
    virtual bool unregisterServiceProvider(ServiceID sid) = 0;
    virtual bool hasServiceProvider(ServiceID sid) = 0;
    virtual bool init(const Address& serverAddr) = 0;
    virtual bool deinit() = 0;
};

}
}
