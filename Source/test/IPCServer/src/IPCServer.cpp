// IPCServer.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <thaf/messaging/Component.h>
#include <thaf/messaging/client-server/ipc/LocalIPCClient.h>
#include <thaf/messaging/Timer.h>
#include <thaf/utils/TimeMeasurement.h>
#include <thaf/messaging/client-server/ipc/LocalIPCServer.h>
#include <thaf/messaging/client-server/ipc/LocalIPCServiceStub.h>
#include "WeatherContract.h"

using namespace thaf::messaging::ipc;
using namespace thaf::messaging;
using namespace thaf::srz;

static const std::string WEATHER_DATA[] = { "Suny", "Windy", "Rainy" };

std::vector<std::string> createBigExtraInfomation(int dataSize)
{
	std::vector<std::string> v = { "nguyen", "van", "con" };
	for (int i = 0; i < dataSize; ++i)
	{
		v.push_back("nguyen ");
	}
	return v;
}

std::vector<std::string> extraInfomation = createBigExtraInfomation(100);
std::string SStatus = "{\"data_id\":\"a4cb90f84d2448009ecc48f6b7ed0c7e\",\"dlp_info\":{},\"file_info\":{\"display_name\":\"componentsplugin4.dll\",\"file_size\":100864,\"file_type\":\"application/x-dosexec\",\"file_type_description\":\"Dynamic Link Library\",\"md5\":\"c713c6f0ea073c1822933aa5be4f1794\",\"sha1\":\"70a4cdf21b39bae2721a0fa84716eca6229ff946\",\"sha256\":\"4644c6f5556414d92eecd3792358fa2ca80a0988469a82f33e928be237420881\",\"upload_timestamp\":\"2019-07-19T03:01:13.471Z\"},\"process_info\":{\"blocked_reason\":\"\",\"file_type_skipped_scan\":false,\"post_processing\":{\"actions_failed\":\"\",\"actions_ran\":\"\",\"converted_destination\":\"\",\"converted_to\":\"\",\"copy_move_destination\":\"\"},\"processing_time\":76,\"profile\":\"File process\",\"progress_percentage\":100,\"queue_time\":7,\"result\":\"Allowed\",\"user_agent\":\"MetaAccess\"},\"scan_results\":{\"data_id\":\"a4cb90f84d2448009ecc48f6b7ed0c7e\",\"progress_percentage\":100,\"scan_all_result_a\":\"No Threat Detected\",\"scan_all_result_i\":0,\"scan_details\":{\"Ahnlab\":{\"def_time\":\"2019-07-19T00:00:00.000Z\",\"eng_id\":\"ahnlab_1_windows\",\"location\":\"local\",\"scan_result_i\":0,\"scan_time\":21,\"threat_found\":\"\",\"wait_time\":7},\"Avira\":{\"def_time\":\"2019-07-17T00:00:00.000Z\",\"eng_id\":\"avira_1_windows\",\"location\":\"local\",\"scan_result_i\":0,\"scan_time\":7,\"threat_found\":\"\",\"wait_time\":7},\"ClamAV\":{\"def_time\":\"2019-07-18T08:12:00.000Z\",\"eng_id\":\"clamav_1_windows\",\"location\":\"local\",\"scan_result_i\":0,\"scan_time\":23,\"threat_found\":\"\",\"wait_time\":11},\"ESET\":{\"def_time\":\"2019-07-18T00:00:00.000Z\",\"eng_id\":\"eset_1_windows\",\"location\":\"local\",\"scan_result_i\":0,\"scan_time\":17,\"threat_found\":\"\",\"wait_time\":11}},\"start_time\":\"2019-07-19T03:01:13.478Z\",\"total_avs\":4,\"total_time\":69},\"vulnerability_info\":{\"verdict\":0},\"yara_info\":{}}";

class ServerTest
{
public:
	ServerTest(bool detached = false) : _comp(detached)
	{

	}

	void start(ServerBase* server, ServiceID sid)
	{
		_comp.onMessage<IPCClientRequestMsg>([this](const MessagePtr<IPCClientRequestMsg>& msg) {
			auto requestKeeper = msg->getRequestKeeper();
			auto result = WeatherStatusResult::create();
			result->props().set_sStatus(SStatus);
			requestKeeper->respond(result);

			for (auto i = 0; i < 100; ++i)
			{
				_stub->sendStatusUpdate(result);
			}

			});

		_comp.start([this, server, sid] {
			_stub = server->createStub<LocalIPCServiceStub>(sid);
			});
	}

private:
	Component _comp;
	std::shared_ptr<LocalIPCServiceStub> _stub;
};
int main()
{
	thafMsg("Server is starting up!");
	auto addr = Address(SERVER_ADDRESS, WEATHER_SERVER_PORT);
	LocalIPCServer server;
	server.init(addr);
	ServerTest s;
	s.start(&server, SID_WeatherService);
	thafMsg("Component shutdown!");
}
