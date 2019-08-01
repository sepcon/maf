#include <thaf/messaging/Component.h>
#include <thaf/messaging/client-server/ipc/LocalIPCClient.h>
#include <thaf/messaging/Timer.h>
#include <thaf/utils/TimeMeasurement.h>
#include <thaf/messaging/client-server/ipc/LocalIPCClient.h>
#include <thaf/messaging/client-server/ipc/LocalIPCServiceProxy.h>
#include "../../IPCServer/src/WeatherContract.h"

using namespace thaf::messaging::ipc;
using namespace thaf::messaging;
using namespace thaf::srz;
using namespace thaf;
 

class ClientCompTest
{
	using IPCProxyPtr = std::shared_ptr<LocalIPCServiceProxy>;
public:
	ClientCompTest(bool detached = false):
		_comp(detached)
	{
	}

	void start(ClientBase* client, ServiceID serviceID)
	{
		_comp.onMessage<ServiceStatusMsg>([this](const MessagePtr<ServiceStatusMsg>& msg) {
			if (msg->newStatus == Availability::Available)
			{
				_proxy->sendStatusChangeRegister<WeatherStatusResult>(CSC_OpID_WeatherStatus, 
					[this](const std::shared_ptr<WeatherStatusResult> result) {
						static int totalUpdate = 0;
						thafMsg("Received result update from server of weather status: " << ++totalUpdate);
					});
			}
			else
			{
				thafMsg("Service is off for sometime, please wait for him to be available again!");
			}
			});
		_comp.start([this, client, serviceID] {
			_proxy = client->createProxy<LocalIPCServiceProxy>(serviceID);
			_proxy->addReference();
			});
	}
private:

	IPCProxyPtr _proxy;
	messaging::Component _comp;
};

int main()
{
	thaf::util::TimeMeasurement tmeasure([](auto time) {
		thafMsg("Total execution time = " << time);
		});
	thafMsg("Client is starting up!");
	auto addr = Address(SERVER_ADDRESS, WEATHER_SERVER_PORT);
	LocalIPCClient client;
	client.init(addr, 500);
	ClientCompTest cl;
	cl.start(&client, SID_WeatherService);
	thafMsg("Client shutdown!");
}
