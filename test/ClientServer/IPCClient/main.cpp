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

    ~ClientCompTest()
    {
        for(auto& regid : _regids)
        {
            if(auto callstatus = _proxy->unregisterBroadcast(regid) != ActionCallStatus::Success)
            {
                Logger::error("Failed to unregister status: ", regid.opID, " with error: ", callstatus);
            }
            else
            {
                Logger::debug("Successfully unregistered status ", regid.opID);
            }
        }
    }

    void onEntry() override
    {
        _proxy = local::createProxy(serverAddress, _sid);
        _timer.setCyclic(true);
        _proxy->setMainComponent(component());

        onMessage<ServiceStatusMsg>([this](ServiceStatusMsg msg) {
            if (msg.newStatus == Availability::Available) {
                maf::Logger::debug("Client component recevies status update of service: " ,  msg.serviceID);
                maf::Logger::debug("Sending requests to server");

                Logger::debug("Server already clear all status, then start jobs...");
                registerStatuses();
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

    void registerStatuses()
    {
        registerStatus<compliance5::status>();
        registerStatus<compliance4::status>();
        registerStatus<compliance3::status>();
        registerStatus<compliance2::status>();
        registerStatus<compliance1::status>();
    }

    template <class Status>
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

        RegID regid = _proxy->registerStatus<Status>(dumpCallback);
        if(regid.valid())
        {
            _regids.push_back(std::move(regid));
        }
        else
        {
            Logger::error("Failed to register property ", Status::operationID());
        }
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
        _proxy->registerSignal<server_request_signal>(
            [](){
            Logger::debug("Received ", server_request_signal::ID,
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
        if(auto lastBootTime = _proxy->getStatus<boot_time::status>())
        {
            Logger::debug("server life is ", lastBootTime->get_seconds());
            if (lastBootTime->get_seconds() > 10)
            {
                if(auto response = _proxy->sendRequest<shutdown>())
                {
                    if(response->isError())
                    {
                        Logger::error("Failed to shutdown server: ", response->getError()->description());
                    }
                    else
                    {
                        Logger::debug("Server already shutdown!");
                    }
                }
            }
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

private:
    Timer _timer;
    std::shared_ptr<Proxy> _proxy;
    std::function<void()> _nextStep;
    std::vector<RegID> _regids;
    ServiceID _sid;
};

#include <iostream>

int main()
{
    std::cout.sync_with_stdio(false);

    maf::Logger::init(
        maf::logging::LOG_LEVEL_DEBUG |
        maf::logging::LOG_LEVEL_FROM_INFO,
        [](const std::string& msg) {
            std::cout << msg << std::endl;
        },
        [](const std::string& msg) {
            std::cerr << msg << std::endl;
        }
        );

    maf::util::TimeMeasurement tmeasure([](auto time) {
        maf::Logger::debug("Total execution time = " ,
                           static_cast<double>(time.count()) / 1000, "ms"
                           );
        });

    maf::Logger::debug("Client is starting up!");
    ClientCompTest cl(SID_WeatherService);
    cl.run();
    maf::Logger::debug("Client shutdown!");


    return 0;
}
