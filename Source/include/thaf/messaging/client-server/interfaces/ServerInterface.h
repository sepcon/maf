#pragma once

#include "CSMessage.h"
#include "CSStatus.h"
#include "CSMessageReceiver.h"

namespace thaf {
namespace messaging {

class ServiceProviderInterface;

class ServerInterface : public CSMessageReceiver
{
public:
    virtual ~ServerInterface() = default;
    virtual DataTransmissionErrorCode sendMessageToClient(const CSMessagePtr& msg, const Address& addr = Address::INVALID_ADDRESS) = 0;
    virtual bool registerServiceProvider(const std::shared_ptr<ServiceProviderInterface>& serviceProvider) = 0;
    virtual bool unregisterServiceProvider(const std::shared_ptr<ServiceProviderInterface>& serviceProvider) = 0;
    virtual bool unregisterServiceProvider(ServiceID sid) = 0;
    virtual bool hasServiceProvider(ServiceID sid) = 0;
};

}
}
