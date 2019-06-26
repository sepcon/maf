#include "thaf/Application/AppComponent.h"
#include "thaf/Application/IPCClient.h"
#include "thaf/Application/Timer.h"
#include "thaf/Application/IPCServer.h"
#include "../../IPCServer/src/WeatherContract.h"


using namespace thaf::messaging::ipc;
using namespace thaf::messaging;
using namespace thaf::app;
using namespace thaf::srz;


struct TimeMeasurement
{
	TimeMeasurement()
	{
		_start = std::chrono::system_clock::now();
	}
	~TimeMeasurement()
	{
		thafMsg("Total execution time: " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - _start).count());
	}
	std::chrono::system_clock::time_point _start;
};



class ClientComp : public AppComponent
{
public:
	~ClientComp()
	{
		shutdown();
		thafMsg("Value of requestDoneCount = " << requestDoneCount);
	}
	void shutdown()
	{
		cl.stop();
		AppComponent::shutdown();
	}

	ClientComp(const Address& addr) : cl(this)
	{
		_detached = false;
		cl.init(IPCType::LocalScope, addr);

		onMessage<ServerConnectionStatusMsg>([this](auto& msg) {
			if (msg->newStatus == ConnectionStatus::Available)
			{
				this->onServerAvailable();
			}
			else
			{
				this->onServerUnavailable();
			}
			});

		start();
	}

	void onServerUnavailable()
	{
		_timer.stop();
	}

	void getWeatherStatus()
	{
		static uint32_t placeid = 0;
		std::shared_ptr<WeatherStatusRequest> request = WeatherStatusRequest::create();
		request->props().set_place_id(placeid++ % 3);

		cl.sendActionRequestSync<WeatherStatusResult>(
			request,
			[this](const std::shared_ptr<WeatherStatusResult>& result) {
				++requestDoneCount;
				thafMsg(std::this_thread::get_id() << ":" << result->props().dump(3));
				if (requestDoneCount >= REQUESTS_PER_CLIENT)
				{
					thafMsg("Posting shutdown request to component");
					shutdown();
				}
			});

	}
	void onServerAvailable()
	{
		_regID = cl.sendStatusChangeRegister<WeatherStatusResult>(CSC_OpID_WeatherStatus, [this](const std::shared_ptr<WeatherStatusResult>& status) {
			++requestDoneCount;
			thafMsg(std::this_thread::get_id() << ":" << status->props().dump(3));
			if (requestDoneCount >= REQUESTS_PER_CLIENT)
			{
				thafMsg("Posting shutdown request to component");
				cl.sendStatusChangeUnregister(_regID);
				shutdown();
			}
			});
		//for (int i = 0; i < REQUESTS_PER_CLIENT; ++i)
		//{
		//	getWeatherStatus();
		//}
	}

	IPCClient cl;
	IPCClient::RegID _regID;
	Timer _timer;
	int requestDoneCount = 0;

};

int main()
{
	TimeMeasurement tmeasure;
	thafMsg("Client is starting up!");
	auto addr = Address(SERVER_ADDRESS, WEATHER_SERVER_PORT);
	ClientComp c(addr);
	thafMsg("Client shutdown!");
}
