#pragma once

#include <maf/messaging/client-server/QueueingServiceStub.h>
#include <maf/messaging/ExtensibleComponent.h>
#include <maf/messaging/CompThread.h>
#include <maf/messaging/Timer.h>
#include <maf/utils/TimeMeasurement.h>
#include "WeatherContract.h"
#include <chrono>

namespace maf {
using namespace messaging;
namespace test {

using namespace weather_service;
using namespace std::chrono;

template <class MessageTrait>
class ServerComponent : public ExtensibleComponent
{
    using Stub = QueueingServiceStub<MessageTrait>;
    template<typename Input>
    using RequestPtrType = typename Stub::template RequestPtrType<Input>;

    std::shared_ptr<Stub> _stub;
    ConnectionType _contype;
public:
    void startTest(
            const ConnectionType& contype,
            const Address& addr
            )
    {
        _stub = Stub::createStub(contype, addr, "weather_service");
        _stub->setMainComponent(component());
        run(LaunchMode::AttachToCurrentThread);
    }

    void stopTest()
    {
        stop();
    }
    void onEntry() override
    {
        using namespace weather_service;
        Logger::debug("Service id is: ", _stub->serviceID());
        _stub->template registerRequestHandler<clear_all_status_request>(
            [this](const auto& request){
                Logger::debug("Received clear status request....");
                this->resetAllStatuses();
                //            request->error("Some error occurred while clearing statuses!");
                request->respond();
            });

        CompThread ( [this] {
            auto success = _stub->template registerRequestHandler<update_status_request>(
                [this](const auto& request) {
                    Logger::debug("Received status update request....");
                    this->setStatus();
                    request->respond();

                    for(int i = 0; i < 10; ++i)
                    {
                        maf::util::TimeMeasurement tm{[](auto elapsedMcs) {
                            Logger::debug(
                                "Time to set compliance 5 status = ",
                                elapsedMcs.count()
                                );
                        }};

                        auto compliance5 = _stub->template getStatus<compliance5::status>();
                        Logger::debug(
                            "The value Server set to client: ",
                            compliance5->dump()
                            );
                    }

                });
            if(!success)
            {
                maf::Logger::debug(
                    "Failed to register request handler for message",
                    update_status_request::operationID());
            }
        }).start().detach();

        _stub->template registerRequestHandler<broad_cast_signal_request>(
            [this](const auto& request){
                request->respond();
                Logger::debug("Received broad cast signal request....");
                this->broadcastSignal();
            });

        _stub->template registerRequestHandler<boot_time>([this](const auto& request){
            using namespace std::chrono;
            Logger::debug("Received boot_time status get request....");
            request->template respond<boot_time::status>(
                duration_cast<seconds>(
                    system_clock::now() - this->_bootTime
                    ).count()
                );
        });

        _stub->template registerRequestHandler<shutdown>(
            [this](const auto& request) {
                static auto retried = 0;

                Logger::debug("Recevied shutdown request from client!");
                if(++retried == 10)
                {
                    Logger::debug("Shutdown server due to many requests from client!");
                    request->respond();
                    stop();
                }
                else
                {
                    request->error("Client is not allowed to shutdown server");
                }
            }
            );

        _stub->template registerRequestHandler<today_weather::input>(
            [](const auto& request) {
                today_weather::input_ptr input = request->getInput();
                Logger::debug(input->dump());
                request->error("today weather is not updated yet!");
            });
        _stub->startServing();
    }

    void onExit() override
    {
        maf::Logger::debug("Server Component is shutting down!");
    }

    void resetAllStatuses()
    {
        _stub->setStatus(compliance5::make_status());
        _stub->setStatus(compliance4::make_status());
        _stub->setStatus(compliance3::make_status());
        _stub->setStatus(compliance2::make_status());
        _stub->setStatus(compliance1::make_status());
        Logger::debug("Clear status done!");
    }

    void setStatus()
    {
        _stub->template setStatus<compliance5::status>(false, true, 0, "hello world");
        _stub->template setStatus<compliance4::status>(false, true, 0, "hello world");
        _stub->template setStatus<compliance3::status>(false, true, 0, "hello world");
        _stub->template setStatus<compliance2::status>(false, true, 0, "hello world");
        _stub->template setStatus<compliance1::status>(false, true, 0, "hello world");
    }

    void broadcastSignal()
    {
        _stub->template broadcastSignal<server_request_signal>();
        _stub->template broadcastSignal<client_info_request::attributes>(
            "nocpes.nocpes"
            );
    }

private:
    system_clock::time_point _bootTime = system_clock::now();
};

}
}

