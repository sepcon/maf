#pragma once

#include "maf/messaging/Timer.h"
#include "maf/utils/TimeMeasurement.h"
#include "maf/messaging/ExtensibleComponent.h"
#include "maf/messaging/client-server/QueueingServiceProxy.h"
//#include "weather_stub_handler.h"

#include "Contract.h"


namespace maf
{
using namespace messaging;

namespace test {

using namespace weather_service;

template<class MessageTrait>
struct ClientComponent : public ExtensibleComponent
{
    using Proxy = QueueingServiceProxy<MessageTrait>;
    std::shared_ptr<Proxy> _proxy;
    Timer _requestTimer;

    ClientComponent(std::shared_ptr<Proxy> proxy) : _proxy{std::move(proxy)} {}
    void startTest()
    {
        _requestTimer.setCyclic(true);
        onMessage<ServiceStatusMsg>([this](const std::shared_ptr<ServiceStatusMsg>& msg) {
            if (msg->newStatus == Availability::Available)
            {
                auto request = today_weather::make_input();
                _proxy->template registerStatus<compliance::status>
                    ([this](const std::shared_ptr<compliance::status>& status){
                        maf::Logger::debug("Component " ,  name() ,
                                           " Got status update from server: \n" ,
                                           status->dump());
                    });

                request->set_command(1);
                auto resultDump = [](const std::shared_ptr<today_weather::output>& result) {
                    maf::Logger::debug(result->dump());
                };

                _proxy->template sendRequestAsync<today_weather::output>( resultDump );

                _requestTimer.start(10, [this, resultDump]{
                    _proxy->template sendRequestAsync<today_weather::output>( resultDump );
                });
            }
            else
            {

            }
        });

        run(LaunchMode::Async);
    }
    void stopTest()
    {
        _proxy.reset();
        stop();
    }

    void onEntry() override
    {
        _proxy->setMainComponent(component());
        maf::Logger::debug("proxy for service 1 ready ! Component  = " ,  name());
    }
};

}
}
