#ifndef MAF_MESSAGING_CLIENT_SERVER_QUEUEINGSERVICEPROXY_H
#define MAF_MESSAGING_CLIENT_SERVER_QUEUEINGSERVICEPROXY_H

#include "CSDefines.h"
#include "ServiceRequesterInterface.h"
#include "ServiceStatusObserverInterface.h"
#include "internal/cs_param.h"
#include <maf/messaging/Component.h>

namespace maf {
namespace messaging {



#define mc_maf_tpl_enable_if_is_base_of_d(base, derive) \
template <class derive, std::enable_if_t<std::is_base_of_v<base, derive>, bool> = true>

#define mc_maf_tpl_enable_if_is_base_of(base, derive) \
template <class derive, std::enable_if_t<std::is_base_of_v<base, derive>, bool>>



template<class MessageTrait>
class QueueingServiceProxy : public ServiceStatusObserverInterface
{
    QueueingServiceProxy(std::shared_ptr<ServiceRequesterInterface> requester);
public:
    static std::shared_ptr<QueueingServiceProxy> createProxy(
            const ConnectionType& contype,
            const Address& addr,
            ServiceID sid
            );

    ServiceID serviceID() const;

    template <class SpecificMsgContent> using PayloadProcessCallback
    = std::function<void(const std::shared_ptr<SpecificMsgContent>&)>;

    template<class Status>
    RegID registerStatus(
            PayloadProcessCallback<Status> callback
            );

    void unregisterStatus(const RegID &regID);
    void unregisterStatusAll(OpID propertyID);

    template<class Status>
    RegID getStatusAsync(
            CSMessageContentHandlerCallback callback
            );

    template<class Status>
    std::shared_ptr<Status> getStatus(
            unsigned long maxWaitTimeMs = maf_MAX_OPERATION_WAIT_MS
            );

    mc_maf_tpl_enable_if_is_base_of_d(cs_result, action_result)
    RegID requestActionAsync
    (
            const std::shared_ptr<cs_request>& requestInput,
            PayloadProcessCallback<action_result> callback = {}
            );

    mc_maf_tpl_enable_if_is_base_of_d(cs_result, action_result)
    RegID requestActionAsync
    (
            PayloadProcessCallback<action_result> callback = {}
            );


    mc_maf_tpl_enable_if_is_base_of_d(cs_request, request_input)
    RegID requestActionAsync
    (
            const std::shared_ptr<request_input>& requestInput
            );


    mc_maf_tpl_enable_if_is_base_of_d(cs_result, action_result)
    std::shared_ptr<action_result> requestAction
    (
            const std::shared_ptr<cs_request>& requestInput,
            unsigned long maxWaitTimeMs = maf_MAX_OPERATION_WAIT_MS
            );


    mc_maf_tpl_enable_if_is_base_of_d(cs_result, action_result)
    std::shared_ptr<action_result> requestAction(
            unsigned long maxWaitTimeMs = maf_MAX_OPERATION_WAIT_MS
            );

    mc_maf_tpl_enable_if_is_base_of_d(cs_request, request_input)
    void requestAction(
            const std::shared_ptr<request_input>& input,
            unsigned long maxWaitTimeMs = maf_MAX_OPERATION_WAIT_MS
            );

    void setMainComponent(ComponentRef compref);

    ~QueueingServiceProxy(){}

protected:
    void onServerStatusChanged(
            Availability oldStatus,
            Availability newStatus
            ) override;
    void onServiceStatusChanged(
            ServiceID sid,
            Availability oldStatus,
            Availability newStatus
            ) override;
    bool updateServiceStatusToComponent(
            Availability oldStatus,
            Availability newStatus
            );
    template<class IncomingMsgContent>
    CSMessageContentHandlerCallback createMsgHandlerAsyncCallback(
            PayloadProcessCallback<IncomingMsgContent> callback
            );

    ComponentRef                                   _compref;
    std::shared_ptr<ServiceRequesterInterface>     _requester;
};


}
}

#ifndef MAF_MESSAGING_CLIENT_SERVER_QUEUEINGSERVICEPROXY_IMPL_H
#   include "internal/QueueingServiceProxy.impl.h"
#endif

#endif
