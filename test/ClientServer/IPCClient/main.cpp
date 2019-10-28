#include <maf/messaging/ExtensibleComponent.h>
#include <maf/messaging/client-server/ipc/LocalIPCClient.h>
#include <maf/messaging/Timer.h>
#include <maf/utils/TimeMeasurement.h>
#include <maf/messaging/client-server/ipc/LocalIPCClient.h>
#include <maf/messaging/client-server/ipc/LocalIPCServiceProxy.h>
#include "../WeatherContract.h"

using namespace maf::messaging::ipc;
using namespace maf::messaging;
using namespace maf::srz;
using namespace maf;


class ClientCompTest : public ExtensibleComponent
{
	using IPCProxyPtr = std::shared_ptr<LocalIPCServiceProxy>;
public:
	ClientCompTest(ServiceID sid) : _sid(sid)
	{
	}

	void onEntry() override
	{
		_proxy = LocalIPCServiceProxy::createProxy(_sid);
        _timer.setCyclic(true);
		onMessage<ServiceStatusMsg>([this](const MessagePtr<ServiceStatusMsg>& msg) {
			if (msg->newStatus == Availability::Available) {
                mafMsg("Client component recevies status update of service: " << msg->serviceID);
                mafMsg("Send request to server");
                _proxy->sendRequest<WeatherStatus::Result>([](const std::shared_ptr<WeatherStatus::Result>& msg) {
                    static int totalResponse = 0;
                    mafMsg("Received update for request of weather status result " << msg->sStatus() << " - " << ++totalResponse);
                    mafMsg(msg->dump());
                });
                _timer.start(SERVER_UPDATE_CYCLE * SERVER_TOTAL_UPDATES_PER_REQUEST + 1000, [this]{
                    _proxy->sendRequest<WeatherStatus::Result>([](const std::shared_ptr<WeatherStatus::Result>& msg) {
                        static int totalResponse = 0;
                        mafMsg("Received update for request of weather status result " << msg->sStatus() << " - " << ++totalResponse);
                        mafMsg(msg->dump());
                    });
                });

			}
			else
			{
				mafMsg("Service is off for sometime, please wait for him to be available again!");
			}
			}
		);
	}
private:
    Timer _timer;
    IPCProxyPtr _proxy;
    ServiceID _sid;
};

#include <iostream>

int main()
{
    maf::debugging::initLogging(maf::debugging::LogLevel::LEVEL_ERROR, [](const std::string& msg) {
        std::cout << msg << std::endl;
    });

	maf::util::TimeMeasurement tmeasure([](auto time) {
		mafMsg("Total execution time = " << time);
        });
	mafMsg("Client is starting up!");
	auto addr = Address(SERVER_ADDRESS, WEATHER_SERVER_PORT);
    LocalIPCClient::instance().init(addr, 10);
	ClientCompTest cl(SID_WeatherService);
	cl.run(LaunchMode::AttachToCurrentThread);
	mafMsg("Client shutdown!");
}
