#include <maf/messaging/ExtensibleComponent.h>
#include <maf/messaging/Timer.h>
#include <maf/messaging/client-server/ipc/LocalIPCServiceProxy.h>
#include <maf/utils/TimeMeasurement.h>
#include "../WeatherContract.h"

using namespace maf::messaging::ipc;
using namespace maf::messaging;
using namespace maf::srz;
using namespace maf;

//using maf::logging::Logger;

static const auto serverAddress = Address{ SERVER_ADDRESS, WEATHER_SERVER_PORT };

using namespace weather_service;

class ClientCompTest : public ExtensibleComponent
{
    using Proxy = local::ServiceProxy;
    using ProxyPtr = std::shared_ptr<Proxy>;
public:
    ClientCompTest(const ServiceID& sid) : _sid(sid)
    {
    }

    void onEntry() override
    {
        _proxy = local::createProxy(serverAddress, _sid);
        _timer.setCyclic(true);
        _proxy->setMainComponent(component());

        onMessage<ServiceStatusMsg>([this](const MessagePtr<ServiceStatusMsg>& msg) {
            if (msg->newStatus == Availability::Available) {
                maf::Logger::debug("Client component recevies status update of service: " ,  msg->serviceID);
                maf::Logger::debug("Sending requests to server");

                _proxy->sendRequest<clear_all_status_request>(static_cast<uint8_t>(-1));
                Logger::debug("Server already clear all status, then start jobs...");

                registerStatus();
                _proxy->sendRequestAsync<update_status_request>();
            }
            else
            {
                maf::Logger::debug("Service is off for sometime, please wait for him to be available again!");
            }
            }
        );
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
                _proxy->sendRequest<struct Category::output>(Category::make_input(), 500);
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
                          status->operationID(), "]: ", status->updated_count());
            if(status->operationID() == compliance1::ID)
            {
                this->getStatuses();
            }
        };

        _proxy->registerStatus<compliance5::status>(dumpCallback);
        _proxy->registerStatus<compliance4::status>(dumpCallback);
        _proxy->registerStatus<compliance3::status>(dumpCallback);
        _proxy->registerStatus<compliance2::status>(dumpCallback);
        _proxy->registerStatus<compliance1::status>(dumpCallback);
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
        _proxy->registerSignal<server_arbittrary_request>([]{
            Logger::debug("Received ", server_arbittrary_request::ID,
                          " from server");
        });

        _proxy->registerSignal<client_info_request::attributes>(
            [this](const auto& signal){
                Logger::debug("Received signal ", client_info_request::ID,
                              "from server: ",
                              signal->dump()
                              );

                tryStopServer();
            });

        _proxy->sendRequestAsync<broad_cast_signal_request>();

    }
    void tryStopServer()
    {
        using namespace weather_service;
        auto lastBootTime = _proxy->getStatus<boot_time::status>();
        Logger::debug("server life is ", lastBootTime->seconds());
        if (lastBootTime->seconds() > 10)
        {
            _proxy->sendRequest<shutdown>();
            Logger::debug("Server already shutdown!");
        }
        stop();
    }
    template <class status>
    void getStatus()
    {
        for(int i = 0; i < 10; ++i)
        {
            auto compliance5 = _proxy->getStatus<status>();
            if(compliance5)
            {
                maf::Logger::debug("Got update from server, status id = ",
                                   status::operationID(),
                                   compliance5->updated_count()
                                   );
            }
            else
            {
                maf::Logger::error("Got empty status from server for status id ",
                                   status::operationID());
            }
        }
    }

private:
    Timer _timer;
    std::shared_ptr<Proxy> _proxy;
    std::function<void()> _nextStep;
    ServiceID _sid;
};

#include <iostream>

int main()
{
    maf::Logger::init(
        maf::logging::LOG_LEVEL_DEBUG |
        maf::logging::LOG_LEVEL_ERROR,
        [](const std::string& msg) {
            std::cout << msg << std::endl;
        }
        );

    maf::util::TimeMeasurement tmeasure([](auto time) {
        maf::Logger::debug("Total execution time = " ,
                           static_cast<double>(time.count()) / 1000, "ms"
                           );
        });

    maf::Logger::debug("Client is starting up!");
    ClientCompTest cl(SID_WeatherService);
    cl.run(LaunchMode::AttachToCurrentThread);
    maf::Logger::debug("Client shutdown!");
}
