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
    using Stub                     = QueueingServiceStub<MessageTrait>;
    using StubPtr                  = std::shared_ptr<Stub>;
public:
    using RequestType               = QueuedRequest<MessageTrait>;
    using RequestPtr                = std::shared_ptr<RequestType>;

    static StubPtr createStub(const ConnectionType& contype, const Address& addr, ServiceID sid);

    template<class property_status>
    ActionCallStatus setStatus(const std::shared_ptr<property_status>& status);

    template<class property_status, typename... Args>
    ActionCallStatus setStatus(Args&&...);

    template<class property_status>
    std::shared_ptr<const property_status> getStatus();

    template <class request_input,
             std::enable_if_t<
                 std::is_base_of_v<cs_input, request_input>, bool> = true
             >
    bool registerRequestHandler(
            std::function<void(RequestPtr,const std::shared_ptr<request_input>&)>
            handlerFunction
            );

    template <class request_class,
             std::enable_if_t<
                 std::is_base_of_v<cs_request, request_class> ||
                 std::is_base_of_v<cs_property, request_class>, bool> = true
             >
    bool registerRequestHandler(
        std::function<void(RequestPtr)> handlerFunction
        );

    bool unregisterRequestHandler( OpID opID );

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
