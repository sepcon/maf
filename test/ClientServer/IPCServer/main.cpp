// IPCServer.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <maf/messaging/ExtensibleComponent.h>
#include <maf/messaging/Timer.h>
#include <maf/messaging/client-server/ipc/LocalIPCClient.h>
#include <maf/utils/TimeMeasurement.h>
#include <maf/messaging/client-server/ipc/LocalIPCServer.h>
#include <maf/messaging/client-server/ipc/LocalIPCServiceStub.h>
#include "../WeatherContract.h"

using namespace maf::messaging::ipc;
using namespace maf::messaging;
using namespace maf::srz;

static const std::string WEATHER_DATA[] = { "Suny", "Windy", "Rainy" };

std::vector<std::string> createBigExtraInfomation(int dataSize)
{
	std::vector<std::string> v = { "nguyen", "van", "con" };
	v.reserve(dataSize);
	for (int i = 0; i < dataSize; ++i)
	{
		v.push_back("nguyen ");
	}
	return v;
}

std::string createBigString(size_t size, const std::string& tobeCloned)
{
	std::string s;
	s.reserve(size * tobeCloned.size());
	for (size_t i = 0; i < size; ++i)
	{
		s += tobeCloned;
	}
	return s;
}
static std::vector<std::string> extraInfomation = createBigExtraInfomation(1000);
static std::string SStatus = createBigString(10, "Hello world"); //"{\"data_id\":\"a4cb90f84d2448009ecc48f6b7ed0c7e\",\"dlp_info\":{},\"file_info\":{\"display_name\":\"componentsplugin4.dll\",\"file_size\":100864,\"file_type\":\"application/x-dosexec\",\"file_type_description\":\"Dynamic Link Library\",\"md5\":\"c713c6f0ea073c1822933aa5be4f1794\",\"sha1\":\"70a4cdf21b39bae2721a0fa84716eca6229ff946\",\"sha256\":\"4644c6f5556414d92eecd3792358fa2ca80a0988469a82f33e928be237420881\",\"upload_timestamp\":\"2019-07-19T03:01:13.471Z\"},\"process_info\":{\"blocked_reason\":\"\",\"file_type_skipped_scan\":false,\"post_processing\":{\"actions_failed\":\"\",\"actions_ran\":\"\",\"converted_destination\":\"\",\"converted_to\":\"\",\"copy_move_destination\":\"\"},\"processing_time\":76,\"profile\":\"File process\",\"progress_percentage\":100,\"queue_time\":7,\"result\":\"Allowed\",\"user_agent\":\"MetaAccess\"},\"scan_results\":{\"data_id\":\"a4cb90f84d2448009ecc48f6b7ed0c7e\",\"progress_percentage\":100,\"scan_all_result_a\":\"No Threat Detected\",\"scan_all_result_i\":0,\"scan_details\":{\"Ahnlab\":{\"def_time\":\"2019-07-19T00:00:00.000Z\",\"eng_id\":\"ahnlab_1_windows\",\"location\":\"local\",\"scan_result_i\":0,\"scan_time\":21,\"threat_found\":\"\",\"wait_time\":7},\"Avira\":{\"def_time\":\"2019-07-17T00:00:00.000Z\",\"eng_id\":\"avira_1_windows\",\"location\":\"local\",\"scan_result_i\":0,\"scan_time\":7,\"threat_found\":\"\",\"wait_time\":7},\"ClamAV\":{\"def_time\":\"2019-07-18T08:12:00.000Z\",\"eng_id\":\"clamav_1_windows\",\"location\":\"local\",\"scan_result_i\":0,\"scan_time\":23,\"threat_found\":\"\",\"wait_time\":11},\"ESET\":{\"def_time\":\"2019-07-18T00:00:00.000Z\",\"eng_id\":\"eset_1_windows\",\"location\":\"local\",\"scan_result_i\":0,\"scan_time\":17,\"threat_found\":\"\",\"wait_time\":11}},\"start_time\":\"2019-07-19T03:01:13.478Z\",\"total_avs\":4,\"total_time\":69},\"vulnerability_info\":{\"verdict\":0},\"yara_info\":{}}";

class ServerComp : public maf::messaging::ExtensibleComponent
{
	ServiceID _sid = ServiceIDInvalid;
public:
	ServerComp(ServiceID sid) : _sid(sid) {}

	void onEntry() override
	{
		_stub = LocalIPCServiceStub::createStub(_sid);
        _updateTimer.setCyclic(true);
        _updateTimer.start(SERVER_UPDATE_CYCLE, [this]{
            for(auto it = std::begin(_keeperResponses);
                 it != _keeperResponses.end(); )
            {
                auto clientId = it - std::begin(_keeperResponses);
                auto result = WeatherStatus::makeResult();
                result->set_sStatus(SStatus);
                result->set_extra_information(extraInfomation);
                result->set_index(it->second);
                mafMsg("Send update to client " << clientId << ": " << it->second++);
                it->first->update(result);
                _totalUpdate++;
                if (it->second == SERVER_TOTAL_UPDATES_PER_REQUEST - 1)
                {
                    _totalUpdate++;
                    result->set_sStatus("100");
                    it->first->respond(result);
                    it = _keeperResponses.erase(it);
                    mafMsg("Complete update for a request ");
                    stop();
                }
                else
                {
                    ++it;
                }
            }
            if(_keeperResponses.size() > 0)
            {
                mafMsg("total updated till now: " << _totalUpdate);
            }
            else
            {
                mafMsg("theres no client request, _total update is now still " << _totalUpdate);
            }
        });
        onMessage<LocalIPCClientRequestMsg>([this](const MessagePtr<LocalIPCClientRequestMsg>& msg) {
			auto requestKeeper = msg->getRequestKeeper();
			switch (requestKeeper->getOperationCode())
			{
            case OpCode::Register:
                mafMsg("Server does not handler for operation register");
				break;
            case OpCode::Request:
                _keeperResponses.emplace_back(std::move(requestKeeper), 0);
				break;
            case OpCode::Abort:
                if (msg->getRequestKeeper() && msg->getRequestKeeper()->getOperationID() == WeatherStatus::ID())
				{
					mafMsg("Receive abort request from clients, but currently no handler");
				}
				break;
			default:
				break;
			}
			});
	}

	void onExit() override
	{
		mafMsg("Server Component is shutting down!");
	}

private:
    std::vector<std::pair<std::shared_ptr<LocalIPCRequestKeeper>, int>> _keeperResponses;
    Timer _updateTimer;
    std::shared_ptr<LocalIPCServiceStub> _stub;
    int _totalUpdate = 0;
	
};
#include <maf/messaging/client-server/ipc/LocalIPCServiceProxy.h>
#include <iostream>

int main()
{
    maf::debugging::initLogging(maf::debugging::LogLevel::LEVEL_ERROR, [](const std::string& msg) {
        std::cout << msg << std::endl;
    });

	mafMsg("Server is starting up!");
	auto addr = Address(SERVER_ADDRESS, WEATHER_SERVER_PORT);
	LocalIPCServer::instance().init(addr);
	ServerComp s(SID_WeatherService);
	s.run(LaunchMode::AttachToCurrentThread);
    mafMsg("Component shutdown!");
}
