#ifndef MAF_MESSAGING_CLIENT_SERVER_QUEUEINGSERVICEPROXY_H
#define MAF_MESSAGING_CLIENT_SERVER_QUEUEINGSERVICEPROXY_H

#include "CSDefines.h"
#include "ResponseT.h"
#include "ServiceRequesterInterface.h"
#include "ServiceStatusObserverInterface.h"
#include "internal/cs_param.h"
#include <maf/messaging/Component.h>

namespace maf {
namespace messaging {


template<class MessageTrait>
class QueueingServiceProxy : public ServiceStatusObserverInterface
{
    QueueingServiceProxy(std::shared_ptr<ServiceRequesterInterface> requester);
public:

    template<class CSParam>
    using ResponseType = ResponseT<CSParam>;

    template<class CSParam>
    using ResponsePtrType = std::shared_ptr<ResponseT<CSParam>>;

    template <class CSParam>
    using ResponseProcessingCallback
        = std::function<void(const ResponsePtrType<CSParam> &)>;

    template <class CSParam>
    using UpdateProcessingCallback
        = std::function<void(const std::shared_ptr<CSParam>&)>;

    static constexpr unsigned long InfiniteWaitPeriod =
        static_cast<unsigned long>(-1);

    static std::shared_ptr<QueueingServiceProxy> createProxy(
            const ConnectionType& contype,
            const Address& addr,
            const ServiceID& sid
            );

    const ServiceID& serviceID() const;
    Availability serviceStatus() const;

    template<class PropertyStatus,
             std::enable_if_t<
                 std::is_base_of_v<cs_status, PropertyStatus>,
                 bool> = true>
    RegID registerStatus(
        UpdateProcessingCallback<PropertyStatus> callback,
        ActionCallStatus* callStatus = nullptr
        );

    template<class SignalAttributes,
             std::enable_if_t<
                 std::is_base_of_v<cs_attributes, SignalAttributes>,
                 bool> = true>
    RegID registerSignal(
        UpdateProcessingCallback<SignalAttributes> callback,
        ActionCallStatus* callStatus = nullptr
        );

    template<class SignalClass,
             std::enable_if_t<
                 std::is_base_of_v<cs_signal, SignalClass>,
                 bool> = true>
    RegID registerSignal(
        std::function<void()> callback,
        ActionCallStatus* callStatus = nullptr
        );

    ActionCallStatus unregisterBroadcast(const RegID &regID);
    ActionCallStatus unregisterBroadcastAll(const OpID& propertyID);

    template<class PropertyStatus,
             std::enable_if_t<
                 std::is_base_of_v<cs_status, PropertyStatus>,
                 bool> = true>
    std::shared_ptr<PropertyStatus> getStatus(
        unsigned long maxWaitTimeMs = InfiniteWaitPeriod,
        ActionCallStatus* callStatus = nullptr
        );

    template <class OperationOrOutput,
             std::enable_if_t<
                 std::is_base_of_v<cs_output, OperationOrOutput> ||
                     std::is_base_of_v<cs_operation, OperationOrOutput>,
                 bool> = true>
    RegID sendRequestAsync(
        const std::shared_ptr<cs_input>& requestInput,
        ResponseProcessingCallback<OperationOrOutput> callback = {},
        ActionCallStatus* callStatus = nullptr
        );

    template <class OperationOrOutput,
             std::enable_if_t<
                 std::is_base_of_v<cs_output, OperationOrOutput> ||
                    std::is_base_of_v<cs_operation, OperationOrOutput>,
                 bool> = true>
    RegID sendRequestAsync(
        ResponseProcessingCallback<OperationOrOutput> callback = {},
        ActionCallStatus* callStatus = nullptr
        );

    template <class OperationOrOutput,
             std::enable_if_t<
                 std::is_base_of_v<cs_output, OperationOrOutput> ||
                 std::is_base_of_v<cs_operation, OperationOrOutput>,
                 bool> = true>
    ResponsePtrType<OperationOrOutput> sendRequest(
        const std::shared_ptr<cs_input>& requestInput,
        unsigned long maxWaitTimeMs = InfiniteWaitPeriod,
        ActionCallStatus* callStatus = nullptr
        );


    template <class OperationOrOutput,
             std::enable_if_t<
                 std::is_base_of_v<cs_output, OperationOrOutput> ||
                     std::is_base_of_v<cs_operation, OperationOrOutput>,
                 bool> = true>
    ResponsePtrType<OperationOrOutput> sendRequest(
        unsigned long maxWaitTimeMs = InfiniteWaitPeriod,
        ActionCallStatus* callStatus = nullptr
        );

    void setMainComponent(ComponentRef compref);

    ~QueueingServiceProxy(){}

private:
    void onServerStatusChanged(
            Availability oldStatus,
            Availability newStatus
            ) override;
    void onServiceStatusChanged(
            const ServiceID& sid,
            Availability oldStatus,
            Availability newStatus
            ) override;
    bool updateServiceStatusToComponent(
            Availability oldStatus,
            Availability newStatus
            );

    template<class CSParam>
    CSMessageContentHandlerCallback createUpdateMsgHandlerCallback(
            UpdateProcessingCallback<CSParam> callback
            );

    template<class CSParam>
    CSMessageContentHandlerCallback createResponseMsgHandlerCallback(
        ResponseProcessingCallback<CSParam> callback
        );

    template<class OperationOrOutput>
    ResponsePtrType<OperationOrOutput> sendRequest(
        OpID actionID,
        const CSMsgContentBasePtr& requestInput,
        unsigned long maxWaitTimeMs,
        ActionCallStatus* callStatus
        );

    template<class CSParam>
    static ResponsePtrType<CSParam> getResposne(const CSMsgContentBasePtr&);

    template<class CSParam>
    static std::shared_ptr<CSParam> getOutput(const CSMsgContentBasePtr&);


    ComponentRef                                   _compref;
    std::shared_ptr<ServiceRequesterInterface>     _requester;
};

template<class MessageTrait>
Availability QueueingServiceProxy<MessageTrait>::serviceStatus() const
{
    return _requester->serviceStatus();
}



}
}

#ifndef MAF_MESSAGING_CLIENT_SERVER_QUEUEINGSERVICEPROXY_IMPL_H
#   include "internal/QueueingServiceProxy.impl.h"
#endif

#endif
