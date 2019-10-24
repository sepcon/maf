#pragma once

#include "maf/messaging/Timer.h"
#include "maf/utils/debugging/Debug.h"
#include "maf/utils/TimeMeasurement.h"
#include "maf/messaging/ExtensibleComponent.h"
#include "maf/messaging/client-server/SCQServiceProxy.h"
#include "Contract.h"


namespace maf
{
using namespace messaging;

namespace test {

template<class MessageTrait, class Client, int ServiceID>
struct ClientComponent : public ExtensibleComponent
{
    using Proxy = SCQServiceProxy<MessageTrait, Client>;
    std::shared_ptr<Proxy> _proxy;
    void startTest()
    {
        onMessage<ServiceStatusMsg>([this](const std::shared_ptr<ServiceStatusMsg>& msg) {
            if (msg->newStatus == Availability::Available)
            {
                auto request = WeatherStatus::makeRequest();
                _proxy->template sendStatusChangeRegister<WeatherStatus::Result>
                    ([this](const std::shared_ptr<WeatherStatus::Result>& result){
                        mafMsg("Component " << name() << " Got Status update from server: \n" << result->dump());
                    });

                request->set_command(1);
                _proxy->template sendRequest<WeatherStatus::Result>([](const std::shared_ptr<WeatherStatus::Result>& result) {
                    mafMsg(result->dump());
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
        _proxy = Proxy::createProxy(ServiceID);
        mafMsg("proxy for service 1 ready ! Component  = " << name());
    }
};

}
}
