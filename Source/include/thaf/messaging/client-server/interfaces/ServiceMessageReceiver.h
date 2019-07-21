#pragma once

#include "CSMessage.h"
#include "CSMessageReceiver.h"


namespace thaf {
namespace messaging {

using CSMessageHandlerCallback = std::function<void (const CSMessagePtr&)>;

class ServiceMessageReceiver : public CSMessageReceiver
{
public:
    const ServiceID &serviceID() const { return _serviceID; }
    void setServiceID(ServiceID serviceID) { _serviceID = std::move(serviceID); }
protected:
    ServiceID _serviceID;
};

} // messaging
} // thaf
