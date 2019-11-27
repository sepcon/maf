#pragma once

#include <maf/messaging/client-server/QueueingServiceStub.h>
#include <maf/messaging/ExtensibleComponent.h>
#include <maf/messaging/Timer.h>
#include <maf/utils/TimeMeasurement.h>
#include "Contract.h"

namespace maf {
using namespace messaging;
namespace test {

using namespace weather_service;

template <class MessageTrait>
class ServerComponent : public ExtensibleComponent
{
    using Stub = QueueingServiceStub<MessageTrait>;
    Timer _serverTimer;
    std::shared_ptr<Stub> _stub;
    ConnectionType _contype;
    Address _serverAddr;
    int _responseCount = 0;
    maf::util::TimeMeasurement::MicroSeconds _totalupdateTime;
public:
    void startTest(
            const ConnectionType& contype,
            const Address& addr,
            const ServiceID& sid = "weather_service"
            )
    {
        _stub = Stub::createStub(contype, addr, sid);
        _stub->setMainComponent(component());
        run(LaunchMode::AttachToCurrentThread);
    }
    void stopTest()
    {
        this->stop();
    }

    void onEntry() override
    {
        maf::Logger::debug("Component is starting");
        _serverTimer.setCyclic(true);
        _stub->template registerRequestHandler<today_weather::input>(
            std::bind(&ServerComponent::handleWeatherRequest, this, std::placeholders::_1)
            );

        auto simpleStatus = simple_status::make_status();
        simpleStatus->headers() = {{"g", "d"},{"d", "k"}};
        _stub->setStatus(simpleStatus);
        _stub->template setStatus<simple_status::status>();
        _stub->startServing();
        maf::Logger::debug("Stub ready as well");
    }

    void handleWeatherRequest(const std::shared_ptr<QueuedRequest<MessageTrait>>& request)
    {
        maf::Logger::debug("Component " ,  name() ,  " Timer expired , total request: " ,  ++_responseCount);
        auto req = request->template getInput<today_weather::input>();
        if(req)
        {
            maf::Logger::debug("Receiver request from client: " ,  req->client_name());
        }
        else
        {
            maf::Logger::debug("Get request with no content, operationID=  " ,  request->getOperationID());
        }

        auto res = today_weather::make_output();
        res->set_shared_list_of_places({"hello", "world"});

        {
            maf::util::TimeMeasurement updateMeasure{
                [this](maf::util::TimeMeasurement::MicroSeconds elapsed) {
                    _totalupdateTime += elapsed;
                    Logger::debug("Time for one update = ", elapsed.count(), "microsecond");
                }
            };
            request->respond(res);
        }

        if(_responseCount >= 100)
        {
            maf::Logger::debug("Send final update to client");
            maf::Logger::debug("Avarage time to send an update " ,  " = " ,  static_cast<double>(_totalupdateTime.count()) / static_cast<double>(_responseCount) / 1000 ,  "ms") ;
            this->stop();
        }
    }
};

}
}

