#ifndef MAF_MESSAGING_CLIENT_SERVER_QUEUEINGSERVICEPROXY_H
#define MAF_MESSAGING_CLIENT_SERVER_QUEUEINGSERVICEPROXY_H

#include "CSDefines.h"
#include "ServiceProxyBase.h"
#include "ClientInterface.h"
#include <maf/messaging/Component.h>
#include <maf/messaging/BasicMessages.h>
#include <maf/utils/cppextension/Lockable.h>
#include <set>


namespace maf {
namespace messaging {


template<class MessageTrait>
class QueueingServiceProxy : public ServiceProxyBase
{
    using _MyBase = ServiceProxyBase;
    using ListOfInterestedComponents = nstl::Lockable<std::set<ComponentRef, comprefless>>;
public:
    using ServiceProxyBase::ServiceProxyBase;

    template <class SpecificMsgContent>
    using PayloadProcessCallback = std::function<void(const std::shared_ptr<SpecificMsgContent>&)>;

    template<class IncomingMsgContent>
    RegID sendStatusChangeRegister(PayloadProcessCallback<IncomingMsgContent> callback);

    template<class IncomingMsgContent>
    RegID sendRequest
    (
            const CSMsgContentPtr& outgoingData,
            PayloadProcessCallback<IncomingMsgContent> callback = {}
            );
    template<class IncomingMsgContent>
    RegID sendRequest
    (
            PayloadProcessCallback<IncomingMsgContent> callback = {}
            );

    template<class IncomingMsgContent>
    bool sendRequestSync
    (
            const CSMsgContentPtr& outgoingData,
            PayloadProcessCallback<IncomingMsgContent> callback,
            unsigned long maxWaitTimeMs = maf_MAX_OPERATION_WAIT_MS
            );  

    template<class IncomingMsgContent>
    std::shared_ptr<IncomingMsgContent> sendRequestSync
    (
            const CSMsgContentPtr& outgoingData,
            unsigned long maxWaitTimeMs = maf_MAX_OPERATION_WAIT_MS
            );

    template<class IncomingMsgContent>
    std::shared_ptr<IncomingMsgContent> sendRequestSync(unsigned long maxWaitTimeMs = maf_MAX_OPERATION_WAIT_MS);

    ~QueueingServiceProxy(){}

protected:
    bool updateServiceStatusToComponent(ComponentRef compref, Availability oldStatus, Availability newStatus);
    void addInterestedComponent(ComponentRef compref);
    void onServerStatusChanged(Availability oldStatus, Availability newStatus) override;
    void onServiceStatusChanged(ServiceID sid, Availability oldStatus, Availability newStatus) override;
    template<class IncomingMsgContent>
    CSMessageHandlerCallback createMsgHandlerAsyncCallback(PayloadProcessCallback<IncomingMsgContent> callback);

    ListOfInterestedComponents _components;
};

}
}

#ifndef MAF_MESSAGING_CLIENT_SERVER_QUEUEINGSERVICEPROXY_IMPL_H
#   include "QueueingServiceProxy.impl.h"
#endif

#endif
