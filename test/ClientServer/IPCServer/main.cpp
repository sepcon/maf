#include <maf/messaging/ExtensibleComponent.h>
#include <maf/messaging/Timer.h>
#include <maf/utils/TimeMeasurement.h>
#include <maf/messaging/client-server/ipc/LocalIPCServiceStub.h>
#include "../WeatherContract.h"

using namespace maf::messaging::ipc;
using namespace maf::messaging;

static const std::string WEATHER_DATA[] = { "Suny", "Windy", "Rainy" };

std::vector<std::string> createBigExtraInfomation(size_t dataSize)
{
    std::vector<std::string> v = { "nguyen", "van", "con" };
    v.reserve(dataSize);
    for (size_t i = 0; i < dataSize; ++i)
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
static std::vector<std::string> extraInfomation = createBigExtraInfomation(1);
static std::string SStatus = createBigString(10, "Hello world"); //"{\"data_id\":\"a4cb90f84d2448009ecc48f6b7ed0c7e\",\"dlp_info\":{},\"file_info\":{\"display_name\":\"componentsplugin4.dll\",\"file_size\":100864,\"file_type\":\"application/x-dosexec\",\"file_type_description\":\"Dynamic Link Library\",\"md5\":\"c713c6f0ea073c1822933aa5be4f1794\",\"sha1\":\"70a4cdf21b39bae2721a0fa84716eca6229ff946\",\"sha256\":\"4644c6f5556414d92eecd3792358fa2ca80a0988469a82f33e928be237420881\",\"upload_timestamp\":\"2019-07-19T03:01:13.471Z\"},\"process_info\":{\"blocked_reason\":\"\",\"file_type_skipped_scan\":false,\"post_processing\":{\"actions_failed\":\"\",\"actions_ran\":\"\",\"converted_destination\":\"\",\"converted_to\":\"\",\"copy_move_destination\":\"\"},\"processing_time\":76,\"profile\":\"File process\",\"progress_percentage\":100,\"queue_time\":7,\"output\":\"Allowed\",\"user_agent\":\"MetaAccess\"},\"scan_results\":{\"data_id\":\"a4cb90f84d2448009ecc48f6b7ed0c7e\",\"progress_percentage\":100,\"scan_all_output_a\":\"No Threat Detected\",\"scan_all_output_i\":0,\"scan_details\":{\"Ahnlab\":{\"def_time\":\"2019-07-19T00:00:00.000Z\",\"eng_id\":\"ahnlab_1_windows\",\"location\":\"local\",\"scan_output_i\":0,\"scan_time\":21,\"threat_found\":\"\",\"wait_time\":7},\"Avira\":{\"def_time\":\"2019-07-17T00:00:00.000Z\",\"eng_id\":\"avira_1_windows\",\"location\":\"local\",\"scan_output_i\":0,\"scan_time\":7,\"threat_found\":\"\",\"wait_time\":7},\"ClamAV\":{\"def_time\":\"2019-07-18T08:12:00.000Z\",\"eng_id\":\"clamav_1_windows\",\"location\":\"local\",\"scan_output_i\":0,\"scan_time\":23,\"threat_found\":\"\",\"wait_time\":11},\"ESET\":{\"def_time\":\"2019-07-18T00:00:00.000Z\",\"eng_id\":\"eset_1_windows\",\"location\":\"local\",\"scan_output_i\":0,\"scan_time\":17,\"threat_found\":\"\",\"wait_time\":11}},\"start_time\":\"2019-07-19T03:01:13.478Z\",\"total_avs\":4,\"total_time\":69},\"vulnerability_info\":{\"verdict\":0},\"yara_info\":{}}";

using namespace weather_service;
class ServerComp : public maf::messaging::ExtensibleComponent
{
public:
    ServerComp(const ServiceID& sid)
    {
        _stub = local::createStub({SERVER_ADDRESS, WEATHER_SERVER_PORT}, sid);
        _stub->setMainComponent( component() );
    }

    void onEntry() override
    {
        using namespace weather_service;
        _stub->registerRequestHandler<clear_all_status_request>([this](const auto& request){
            Logger::debug("Received clear status request....");
            this->resetAllStatuses();
            request->respond();
        });

        _stub->registerRequestHandler<update_status_request>([this](const auto& request){
            Logger::debug("Received status update request....");
            this->setStatus();
            request->respond();
            for(int i = 0; i < 10; ++i)
            {
                maf::util::TimeMeasurement tm{[](auto elapsedMcs) {
                    Logger::debug("Time to set compliance 5 status = ", elapsedMcs.count());
                }};

                auto compliance5Status = _stub->getStatus<compliance5::status>();
                Logger::debug(
                    "The value Server set to client: ",
                    compliance5Status->dump()
                    );
            }

        });

        _stub->registerRequestHandler<broad_cast_signal_request>([this](const auto& request){
            request->respond();
            Logger::debug("Received broad cast signal request....");
            this->broadcastSignal();
        });

        _stub->registerRequestHandler<boot_time>([this](const auto& request){
            using namespace std::chrono;
            Logger::debug("Received boot_time status get request....");
            request->template respond<boot_time::status>(
                duration_cast<seconds>(system_clock::now() - this->_bootTime).count()
                );
        });

        _stub->registerRequestHandler<shutdown>([this](const auto& request) {
            Logger::debug("Recevied shutdown request from client!");
            request->respond();
            stop();
            }
        );

        _stub->startServing();
    }

    void onExit() override
    {
        maf::Logger::debug("Server Component is shutting down!");
    }

    void resetAllStatuses()
    {
        _stub->setStatus(compliance5::make_status());
        _stub->setStatus(compliance4::make_status());
        _stub->setStatus(compliance3::make_status());
        _stub->setStatus(compliance2::make_status());
        _stub->setStatus(compliance1::make_status());
        Logger::debug("Clear status done!");
    }

    void setStatus()
    {
        _stub->setStatus<compliance5::status>(false, true, 0, "hello world");
        _stub->setStatus<compliance4::status>(false, true, 0, "hello world");
        _stub->setStatus<compliance3::status>(false, true, 0, "hello world");
        _stub->setStatus<compliance2::status>(false, true, 0, "hello world");
        _stub->setStatus<compliance1::status>(false, true, 0, "hello world");
    }

    void broadcastSignal()
    {
        _stub->broadcastSignal<server_arbittrary_request>();
        _stub->broadcastSignal<client_info_request::attributes>(
            "nocpes.nocpes"
            );
    }

private:
    std::chrono::system_clock::time_point _bootTime = std::chrono::system_clock::now();
    std::shared_ptr<local::ServiceStub> _stub;

};

#include <iostream>

int main()
{
    auto logFunc = [](const std::string& msg) {
        std::cout << msg << std::endl;
    };
    auto errFunc = [](const std::string& msg) {
        std::cerr << msg << std::endl;
    };

    maf::Logger::init(maf::logging::LOG_LEVEL_DEBUG | maf::logging::LOG_LEVEL_FROM_WARN,
                      std::move(logFunc), std::move(errFunc));

    maf::Logger::debug("Server is starting up!");
    ServerComp s(SID_WeatherService);
    s.run(LaunchMode::AttachToCurrentThread);
    maf::Logger::debug("Component shutdown!");

}

