#ifndef MAF_MESSAGING_CLIENT_SERVER_QUEUEINGSERVICESTUB_H
#define MAF_MESSAGING_CLIENT_SERVER_QUEUEINGSERVICESTUB_H

#include <maf/messaging/client-server/ServerFactory.h>
#include <maf/messaging/client-server/QueuedRequest.h>
#include <maf/messaging/Component.h>
#include <maf/messaging/BasicMessages.h>
#include <maf/logging/Logger.h>

namespace maf {
namespace messaging {

/**
 * @brief QueueingServiceStub class provides a generic interface of a ServiceProvider that is tight coupling with
 * a messaging::Component, to help handling Service Message in a queueing maner to prevent issue of data races
 * coping with multithreading application.
 * @class MessageTrait: must provide interfaces of translating specific type messages to CSMessage and vice versa
 */
template <class MessageTrait>
class QueueingServiceStub final
{
    using Stub                    = QueueingServiceStub<MessageTrait>;
    using StubPtr                 = std::shared_ptr<Stub>;
public:
    template <typename InputType  = nullptr_t>
    using RequestType             = QueuedRequest<MessageTrait, InputType>;
    template <typename InputType  = nullptr_t>
    using RequestPtrType          = std::shared_ptr<RequestType<InputType>>;
    template <typename InputType  = nullptr_t>
    using RequestHandlerFunction  = std::function<
                                        void(const RequestPtrType<InputType>& )
                                    >;

    static StubPtr createStub(const ConnectionType& contype, const Address& addr, const ServiceID& sid);

    const ServiceID& serviceID() const;

    template<class property_status,
             std::enable_if_t<
                 std::is_base_of_v<cs_status, property_status>,
                 bool> = true>
    ActionCallStatus setStatus(const std::shared_ptr<property_status>& status);

    template<class property_status, typename... Args,
             std::enable_if_t<
                 std::is_base_of_v<cs_status, property_status>,
                 bool> = true>
    ActionCallStatus setStatus(Args&&...);

    template<class property_status,
             std::enable_if_t<
                 std::is_base_of_v<cs_status, property_status>,
                 bool> = true>
    std::shared_ptr<const property_status> getStatus();

    template<class signal_attributes,
             std::enable_if_t<
                 std::is_base_of_v<cs_attributes, signal_attributes>,
                 bool> = true>
    ActionCallStatus broadcastSignal(
        const std::shared_ptr<signal_attributes>& attr
        );

    template<class signal_attributes, typename... Args,
             std::enable_if_t<
                 std::is_base_of_v<cs_attributes, signal_attributes>,
                 bool> = true>
    ActionCallStatus broadcastSignal(Args&&... args);

    template<class signal_class,
             std::enable_if_t<
                 std::is_base_of_v<cs_signal, signal_class>,
                 bool> = true>
    ActionCallStatus broadcastSignal();

    template <class RequestInput,
             std::enable_if_t<
                 std::is_base_of_v<cs_input, RequestInput> ||
                 std::is_base_of_v<cs_operation, RequestInput>,
                 bool> = true
             >
    bool registerRequestHandler(
        RequestHandlerFunction<RequestInput> handlerFunction
        );

    bool unregisterRequestHandler( const OpID& opID );

    void setMainComponent(ComponentRef copmref);

    void startServing();
    void stopServing();

private:

    QueueingServiceStub(std::shared_ptr<ServiceProviderInterface> provider);
    bool getHandlerComponent(ComponentRef &compref) const;
    void onComponentUnavailable();

    std::shared_ptr<ServiceProviderInterface> _provider;
    ComponentRef _compref;
};


}
}

#ifndef MAF_MESSAGING_CLIENT_SERVER_QUEUEINGSERVICESTUB_IMPL_H
#   include "internal/QueueingServiceStub.Impl.h"
#endif

#endif
