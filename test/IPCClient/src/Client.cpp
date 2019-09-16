#include <maf/messaging/ExtensibleComponent.h>
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
		onMessage<ServiceStatusMsg>([this](const MessagePtr<ServiceStatusMsg>& msg) {
			if (msg->newStatus == Availability::Available) {
				mafMsg("Client component recevies status update of service: " << msg->serviceID);
				static bool statusReg = true;
				//if (statusReg)
				//{
				//	mafMsg("Send Status change register to server");
				//	_proxy->sendStatusChangeRegister<WeatherStatusResult>(CSC_OpID_WeatherStatus,
				//		[this](const std::shared_ptr<WeatherStatusResult> result) {
				//			static int totalUpdate = 0;
				//			mafMsg("Received result update from server of weather status: " << ++totalUpdate);
				//		});

				//}
				//else
				{
					mafMsg("Send request to server");
					auto regid = _proxy->sendRequest<WeatherStatusResult>([](const std::shared_ptr<WeatherStatusResult>& msg) {
						static int totalResponse = 0;
						mafMsg("Received update for request of weather status result " << msg->props().sStatus() << " - " << ++totalResponse);
						mafMsg(msg->props().dump());
						});
					_timer.start(500, [this, regid] {
						mafMsg("send abort the request immediately");
						//_proxy->sendAbortRequest(regid);
						_proxy->sendRequest<ShutDownServerRequestResult>();
						});
				}

				statusReg = !statusReg;
			}
			else
			{
				mafMsg("Service is off for sometime, please wait for him to be available again!");
			}
			}
		);
	}
private:
	ServiceID _sid;
	Timer _timer;
	IPCProxyPtr _proxy;
};

int main()
{
	auto proxy = LocalIPCServiceProxy::createProxy(1);
	maf::util::TimeMeasurement tmeasure([](auto time) {
		mafMsg("Total execution time = " << time);
		});
	mafMsg("Client is starting up!");
	auto addr = Address(SERVER_ADDRESS, WEATHER_SERVER_PORT);
	LocalIPCClient::instance().init(addr, 500);
	ClientCompTest cl(SID_WeatherService);
	cl.run(LaunchMode::AttachToCurrentThread);
	mafMsg("Client shutdown!");
}
