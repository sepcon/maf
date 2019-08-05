#include <maf/messaging/Component.h>
#include <maf/messaging/client-server/ipc/LocalIPCClient.h>
#include <maf/messaging/Timer.h>
#include <maf/utils/TimeMeasurement.h>
#include <maf/messaging/client-server/ipc/LocalIPCClient.h>
#include <maf/messaging/client-server/ipc/LocalIPCServiceProxy.h>
#include "../../IPCServer/src/WeatherContract.h"

using namespace maf::messaging::ipc;
using namespace maf::messaging;
using namespace maf::srz;
using namespace maf;
 

class ClientCompTest
{
	using IPCProxyPtr = std::shared_ptr<LocalIPCServiceProxy>;
public:
	ClientCompTest(bool detached = false):
		_comp(detached)
	{
	}

	void start(ServiceID serviceID)
	{
		_comp.onMessage<ServiceStatusMsg>([this](const MessagePtr<ServiceStatusMsg>& msg) {
			if (msg->newStatus == Availability::Available)
			{
				mafMsg("Client component recevies status update of service: " << msg->serviceID);
				static bool statusReg = true;
				//if (statusReg)
				{
					mafMsg("Send Status change register to server");
					_proxy->sendStatusChangeRegister<WeatherStatusResult>(CSC_OpID_WeatherStatus,
						[this](const std::shared_ptr<WeatherStatusResult> result) {
							static int totalUpdate = 0;
							mafMsg("Received result update from server of weather status: " << ++totalUpdate);
						});
				}
				/*else
				{
					mafMsg("Send request to server");
					_proxy->sendRequest<WeatherStatusResult>([](const std::shared_ptr<WeatherStatusResult>& msg){
						static int totalResponse = 0;
						mafMsg("Received update for request of weather status result " << msg->props().get_sStatus() << " - " << ++totalResponse);
						});
				}*/

				statusReg = !statusReg;
			}
			else
			{
				mafMsg("Service is off for sometime, please wait for him to be available again!");
			}
			});
		_comp.start([this, serviceID] {
			_proxy = LocalIPCServiceProxy::createProxy(serviceID);
			});
	}
private:

	IPCProxyPtr _proxy;
	messaging::Component _comp;
};

int main()
{
	maf::util::TimeMeasurement tmeasure([](auto time) {
		mafMsg("Total execution time = " << time);
		});
	mafMsg("Client is starting up!");
	auto addr = Address(SERVER_ADDRESS, WEATHER_SERVER_PORT);
	LocalIPCClient::instance().init(addr, 500);
	ClientCompTest cl;
	cl.start(SID_WeatherService);
	mafMsg("Client shutdown!");
}
