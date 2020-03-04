#pragma once

#include "maf/messaging/Timer.h"
#include "maf/utils/TimeMeasurement.h"
#include "maf/messaging/ExtensibleComponent.h"
#include "maf/messaging/client-server/QueueingServiceProxy.h"
#include "WeatherContract.h"
#include <thread>

using namespace std::chrono_literals;

namespace maf
{
using namespace messaging;

namespace test {

using namespace weather_service;

template<class MessageTrait>
struct ClientComponent : public ExtensibleComponent
{
    using Proxy = QueueingServiceProxy<MessageTrait>;

    template <class CSParam>
    using ResponsePtr = typename Proxy::template ResponsePtrType<CSParam>;
private:
    std::shared_ptr<Proxy> _proxy;
    Timer _timer;

public:
    ClientComponent(std::shared_ptr<Proxy> proxy) : _proxy{std::move(proxy)} {}

    void startTest(
        const ConnectionType& contype,
        const Address& addr
        )
    {
        _proxy = Proxy::createProxy( contype, addr, "weather_service");
        _timer.setCyclic(true);
        _proxy->setMainComponent(component());
        onMessage<ServiceStatusMsg>([this](const MessagePtr<ServiceStatusMsg>& msg) {
            if (msg->newStatus == Availability::Available) {
                maf::Logger::debug("Client component recevies status update of service: " ,  msg->serviceID);
                maf::Logger::debug("Sending requests to server");

                auto response = _proxy->template sendRequest<clear_all_status_request>(1000ms);
                if(auto error = response->getError())
                {
                    Logger::error("Failed on request ",
                                  update_status_request::ID,
                                  ": ", error->description()
                                  );
                    Component::getActiveSharedPtr()->stop();
                    return;
                }
                else
                {
                    auto output = response->getOutput();
                    Logger::debug("Received response for id: ", output->ID);
                }

                Logger::debug("Server already clear all status, then start jobs...");
                registerStatus();
                _proxy->template sendRequestAsync<update_status_request>();
            }
            else
            {
                maf::Logger::debug("Service is off for sometime, please wait for him to be available again!");
            }
        });
        run(LaunchMode::Async);
    }
    void stopTest()
    {
        _proxy.reset();
        stop();
    }

    template <typename Category>
    void sendSyncRequest()
    {
        long long total = 0;
        const int totalRequests = 5000;
        {
            util::TimeMeasurement tm{[&total](util::TimeMeasurement::MicroSeconds elapsed) {
                total += elapsed.count();
            }};
            for(int i = 0; i < totalRequests; ++i)
            {
                try
                {
                    _proxy->template sendRequest<struct Category::output>(Category::make_input(), 500);
                }
                catch(const std::exception& e)
                {

                }
            }
        }
        auto avarageTimePerRequest = static_cast<double>(total) / totalRequests;
        Logger::debug("Avarage time to send a request is: ",
                      avarageTimePerRequest,
                      " microseconds");
    }

    void registerStatus()
    {
        auto dumpCallback = [this](const auto& status) {
            Logger::debug("Got status update from server[",
                          status->operationID(), "]: ", status->get_updated_count());
            if(status->operationID() == compliance1::ID)
            {
                this->getStatuses();
            }
        };

        _proxy->template registerStatus<compliance5::status>(dumpCallback);
        _proxy->template registerStatus<compliance4::status>(dumpCallback);
        _proxy->template registerStatus<compliance3::status>(dumpCallback);
        _proxy->template registerStatus<compliance2::status>(dumpCallback);
        _proxy->template registerStatus<compliance1::status>(dumpCallback);
    }

    void getStatuses()
    {
        getStatus<compliance::status>();
        getStatus<compliance5::status>();
        getStatus<compliance1::status>();
        registerSignal();
    }

    void registerSignal()
    {
        _proxy->template registerSignal<server_request_signal>(
            [](){
                Logger::debug("Received ", server_request_signal::ID,
                              " from server");
            });

        _proxy->template registerSignal<client_info_request::attributes>(
            [this](const auto& signal){
                Logger::debug("Received signal ", client_info_request::ID,
                              "from server: ",
                              signal->dump()
                              );

                tryStopServer();
            });

        _proxy->template sendRequestAsync<broad_cast_signal_request>();

    }
    void tryStopServer()
    {
        using namespace weather_service;
		uint64_t bootTime = 0;
		do
		{
			if (auto lastBootTime = _proxy->template getStatus<boot_time::status>())
			{
				Logger::debug("server life is ", lastBootTime->get_seconds());

				if (auto response = _proxy->template sendRequest<shutdown>())
				{
					if (response->isError())
					{
						Logger::error("Failed to shutdown server: ", response->getError()->description());
					}
					else
					{
						Logger::debug("Server already shutdown!");
						break;
					}
				}
				std::this_thread::sleep_for(std::chrono::seconds(1));

			}
			else
			{
				Logger::debug("Server is not working now, then exit!");
				break;
			}
		} while (bootTime < 10);
		
        stop();
    }
    template <class status>
    void getStatus()
    {
        for(int i = 0; i < 10; ++i)
        {
            auto compliance5 = _proxy->template getStatus<status>();
            if(compliance5)
            {
                maf::Logger::debug("Got update from server, status id = ",
                                   status::operationID(),
                                   compliance5->get_updated_count()
                                   );
            }
            else
            {
                maf::Logger::error("Got empty status from server for status id ",
                                   status::operationID());
            }
        }
    }

};

}
}
