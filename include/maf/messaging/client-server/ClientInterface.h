#pragma once

#include "CSStatus.h"
#include "CSMessageReceiver.h"
#include "ServiceRequesterInterface.h"
//#include "ServiceStatusObserverInterface.h"

namespace maf {
namespace messaging {

using ServiceRequesterInterfacePtr = std::shared_ptr<ServiceRequesterInterface>;

class ClientInterface : public CSMessageReceiver, private ServiceStatusObserverInterface
{
public:
    virtual ~ClientInterface() = default;
    virtual ActionCallStatus sendMessageToServer(const CSMessagePtr& msg) = 0;
    virtual bool hasServiceRequester(ServiceID sid) = 0;
    virtual ServiceRequesterInterfacePtr getServiceRequester(ServiceID sid) = 0;
    virtual Availability getServiceStatus(ServiceID sid) = 0;
    virtual bool init(const Address& serverAddr, long long sersverMonitoringCycleMS) = 0;
    virtual bool deinit() = 0;
};

}
}
