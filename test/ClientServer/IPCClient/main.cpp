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

using namespace weather_contract;

class ClientCompTest : public ExtensibleComponent
{
    using Proxy = local::ServiceProxy;
    using ProxyPtr = std::shared_ptr<Proxy>;
public:
    ClientCompTest(ServiceID sid) : _sid(sid)
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

                registerStatus();

                _timer.start(1000, [this]{
                    static int totalRequest = 0;
                    _proxy->requestActionAsync<weather_contract::today_weather::result>(
                        weather_contract::today_weather::make_request("this is ipc client ", ++totalRequest),
                        [this](const weather_contract::today_weather::result_ptr& result) {
                            Logger::debug("Server resonds: ", result->dump());
                            if(result->your_command() >= 5) {
                                getStatuses();
                                this->sendSyncRequest<weather_contract::today_weather>();
                                tryStopServer();
                                this->stop();
                            }
                        } );
                });

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
        for(int i = 0; i < 50; ++i)
        {
            util::TimeMeasurement tm{[](util::TimeMeasurement::MicroSeconds elapsed) {
                Logger::debug("Totol time for request sync = ", elapsed.count(), " microseconds");
            }};

            if(auto result = _proxy->requestAction<struct Category::result>(Category::make_request(), 500))
                Logger::debug("Receive result from server for sync request = ", result->dump());
            else
            {
                Logger::debug("Action result from server is failed");
            }
        }
    }

    void registerStatus()
    {
        auto dumpCallback = [](const auto& status) {
            Logger::debug(status->dump());
        };
        _proxy->registerStatus<weather_contract::compliance::status>(dumpCallback);
        _proxy->registerStatus<weather_contract::compliance5::status>(dumpCallback);
        _proxy->registerStatus<weather_contract::compliance1::status>(dumpCallback);
    }

    void getStatuses()
    {
        getStatus<weather_contract::compliance::status>();
        getStatus<weather_contract::compliance5::status>();
        getStatus<weather_contract::compliance1::status>();
    }

    void tryStopServer()
    {
        using namespace weather_contract;
        auto lastBootTime = _proxy->getStatus<boot_time::status>();
        Logger::debug("server life is ", lastBootTime->seconds());
        if (lastBootTime->seconds() > 10)
        {
            _proxy->requestAction<shutdown::request>({});
            Logger::debug("Server already shutdown!");
        }
    }
    template <class status>
    void getStatus()
    {
        for(int i = 0; i < 10; ++i)
        {
            auto compliance5 = _proxy->getStatus<status>();
            if(compliance5)
            {
                maf::Logger::debug("Got update from server: ", compliance5->dump(-1));
            }
            else
            {
                maf::Logger::error("Got empty status from server for status id ", status::operationID());
            }
        }
    }

private:
    Timer _timer;
    std::shared_ptr<Proxy> _proxy;
    ServiceID _sid;
};

#include <iostream>

int main()
{
    maf::Logger::init(maf::logging::LOG_LEVEL_DEBUG | maf::logging::LOG_LEVEL_ERROR, [](const std::string& msg) {
        std::cout << msg << std::endl;
    });

    maf::util::TimeMeasurement tmeasure([](auto time) {
        maf::Logger::debug("Total execution time = " ,  static_cast<double>(time.count()) / 1000, "ms");
        });

    maf::Logger::debug("Client is starting up!");
    ClientCompTest cl(SID_WeatherService);
    cl.run(LaunchMode::AttachToCurrentThread);
    maf::Logger::debug("Client shutdown!");
}
