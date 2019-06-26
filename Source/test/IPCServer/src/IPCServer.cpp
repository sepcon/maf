// IPCServer.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "thaf/Application/AppComponent.h"
#include "thaf/Application/Timer.h"
#include "thaf/Application/IPCServer.h"
#include "thaf/Application/BasicMessages.h"
#include "thaf/Application/IPCMessages.h"
#include "WeatherContract.h"

using namespace thaf::messaging::ipc;
using namespace thaf::messaging;
using namespace thaf::app;
using namespace thaf::srz;

static const std::string WEATHER_DATA[] = { "Suny", "Windy", "Rainy" };

std::vector<std::string> createBigExtraInfomation()
{
	std::vector<std::string> v = { "nguyen", "van", "con" };
	for (int i = 0; i < 20; ++i)
	{
		v.push_back("nguyen ");
	}
	return v;
}

std::vector<std::string> extraInfomation = createBigExtraInfomation();
std::string SStatus = "{\"data_id\":\"a4cb90f84d2448009ecc48f6b7ed0c7e\",\"dlp_info\":{},\"file_info\":{\"display_name\":\"componentsplugin4.dll\",\"file_size\":100864,\"file_type\":\"application/x-dosexec\",\"file_type_description\":\"Dynamic Link Library\",\"md5\":\"c713c6f0ea073c1822933aa5be4f1794\",\"sha1\":\"70a4cdf21b39bae2721a0fa84716eca6229ff946\",\"sha256\":\"4644c6f5556414d92eecd3792358fa2ca80a0988469a82f33e928be237420881\",\"upload_timestamp\":\"2019-07-19T03:01:13.471Z\"},\"process_info\":{\"blocked_reason\":\"\",\"file_type_skipped_scan\":false,\"post_processing\":{\"actions_failed\":\"\",\"actions_ran\":\"\",\"converted_destination\":\"\",\"converted_to\":\"\",\"copy_move_destination\":\"\"},\"processing_time\":76,\"profile\":\"File process\",\"progress_percentage\":100,\"queue_time\":7,\"result\":\"Allowed\",\"user_agent\":\"MetaAccess\"},\"scan_results\":{\"data_id\":\"a4cb90f84d2448009ecc48f6b7ed0c7e\",\"progress_percentage\":100,\"scan_all_result_a\":\"No Threat Detected\",\"scan_all_result_i\":0,\"scan_details\":{\"Ahnlab\":{\"def_time\":\"2019-07-19T00:00:00.000Z\",\"eng_id\":\"ahnlab_1_windows\",\"location\":\"local\",\"scan_result_i\":0,\"scan_time\":21,\"threat_found\":\"\",\"wait_time\":7},\"Avira\":{\"def_time\":\"2019-07-17T00:00:00.000Z\",\"eng_id\":\"avira_1_windows\",\"location\":\"local\",\"scan_result_i\":0,\"scan_time\":7,\"threat_found\":\"\",\"wait_time\":7},\"ClamAV\":{\"def_time\":\"2019-07-18T08:12:00.000Z\",\"eng_id\":\"clamav_1_windows\",\"location\":\"local\",\"scan_result_i\":0,\"scan_time\":23,\"threat_found\":\"\",\"wait_time\":11},\"ESET\":{\"def_time\":\"2019-07-18T00:00:00.000Z\",\"eng_id\":\"eset_1_windows\",\"location\":\"local\",\"scan_result_i\":0,\"scan_time\":17,\"threat_found\":\"\",\"wait_time\":11}},\"start_time\":\"2019-07-19T03:01:13.478Z\",\"total_avs\":4,\"total_time\":69},\"vulnerability_info\":{\"verdict\":0},\"yara_info\":{}}";
#include <fstream>
struct ServerComp : public AppComponent
{

	static const std::string PossibStatus[];
public:
	void shutdown()
	{
		AppComponent::shutdown();
		thafMsg("Component shutdown successfully, then take a short rest of 2 seconds");
		for (int i = 0; i < 1000; ++i)
		{
			thafMsg("Trying to output as much as possible before leaving the world");
		}
		thafMsg("Component wakes up from the rest!, then leaves the world immediately");
	}

	ServerComp(const Address& addr) : sv(this)
	{
		_detached = false;
		std::ofstream f("D:\\ServerComp.log");

		sv.init(IPCType::LocalScope, addr);
		
		onSignal<ShutdownMessage>([this] {
			std::remove("D:\\ServerComp.log");
			this->shutdown();
			});
		
		onMessage<IPCClientRequestMessage>([this](CMessagePtr<IPCClientRequestMessage>& msg) {

			auto replier = msg->getRequestTracker();
			static size_t requestCount = 1;
			thafMsg("----------->Got request: " << requestCount++);

			_timer.start(3000, [this] {
				for (int i = 0; i < REQUESTS_PER_CLIENT; ++i)
				{
					thafMsg("Start sending weather status to clients: " << requestCount);
					auto status = WeatherStatusResult::create();
					status->props().set_extra_information(extraInfomation);
					(*status)->set_sStatus(SStatus);
					sv.sendStatusUpdate(status);
				}
				});

			if (replier->getOpCode() == OpCode::Register && replier->getOpID() == CSC_OpID_WeatherStatus)
			{
				
			}
			else if (replier->getOpID() == CSC_OpID_WeatherStatus && replier->getOpCode() == OpCode::Request)
			{

				auto weatherStatusRequest = replier->getDataCarrier<WeatherStatusRequest>();
				if (weatherStatusRequest)
				{
					auto status = WeatherStatusResult::create();
					status->props().set_status(static_cast<WeatherStatusResult::StatusType>(weatherStatusRequest->props().get_place_id()));
					status->props().set_extra_information(extraInfomation);
					(*status)->set_sStatus(SStatus);
					replier->reply(status);

				}
			} 
			});
	}

	void constantlyUpdateWeatherStatus()
	{
		for (int i = 0; i < 20; ++i)
		{
			auto status = WeatherStatusResult::create();
			status->props().set_status(static_cast<WeatherStatusResult::StatusType>(i % 3));
			status->props().set_extra_information(extraInfomation);
			sv.sendStatusUpdate(status);
		}
	}
	void run()
	{
		start(std::bind(&ServerComp::entryFunction, this));
	}

	void entryFunction()
	{
		_timer.start(1000, [] {thafMsg("Timer expired!"); });
		for (int i = 0; i < 3; ++i)
		{
			_timer.start(2000, [] {thafMsg("Timer expired!"); });
		}
	}
	IPCServer sv;
	Timer _timer;
	int count = 0;
};

const std::string ServerComp::PossibStatus[] = { "Rainy", "Suny", "Windy" };


int main()
{
	thafMsg("Server is starting up!");
	auto addr = Address(SERVER_ADDRESS, WEATHER_SERVER_PORT);
	ServerComp s(addr);
	s.run();
	thafMsg("Component shutdown!");
}
