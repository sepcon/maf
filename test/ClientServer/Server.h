#pragma once

#include <maf/messaging/client-server/SSQServiceStub.h>
#include <maf/messaging/ExtensibleComponent.h>
#include <maf/messaging/Timer.h>
#include <maf/utils/TimeMeasurement.h>
#include "Contract.h"

namespace maf {
using namespace messaging;
namespace test {


template <class MessageTrait, class Server, int ServiceID>
class ServerComponent : public ExtensibleComponent
{
    using RequestMessage = ClientRequestMessage<MessageTrait>;
    using Stub = SSQServiceStub<MessageTrait, Server>;
    std::shared_ptr<Stub> _stub;
    Timer _serverTimer;

public:
    void startTest(bool detached = true)
    {
        _serverTimer.setCyclic(true);
        onMessage<RequestMessage>([this](const std::shared_ptr<RequestMessage>& msg) {
            auto requestKeeper = msg->getRequestKeeper();
            switch(requestKeeper->getOperationCode())
            {
            case OpCode::Register:
            {
                mafMsg("Client registered to status " << msg->getRequestKeeper()->getOperationID());
                auto msg = WeatherStatus::makeResult();
                msg->set_the_status("Response to register");
                requestKeeper->respond(msg);
            }
            break;
            case OpCode::Request:
            {
                _serverTimer.start(10, [this, requestKeeper] {
                    static thread_local int responseCount = 0;
                    static thread_local long long totalupdateTime = 0;

                    mafMsg("Component " << name() << " Timer expired , total request: " << ++responseCount);
                    auto res = std::make_shared<WeatherStatus::Result>();
                    res->set_shared_list_of_places({"hello", "world"});

                    {
                        maf::util::TimeMeasurement updateMeasure{
                            [](long long elapsed) {
                                totalupdateTime += elapsed;
                            }
                        };

                        requestKeeper->update(res);
                    }
                    if(responseCount >= 100)
                    {
                        mafMsg("Send final update to client");
                        mafMsg("Avarage time to send an update " << " = " << static_cast<double>(totalupdateTime) / static_cast<double>(responseCount) << "ms") ;
                        requestKeeper->respond(res);
                        _serverTimer.stop();
                        this->stop();
                    }
                });
                auto req = msg->template getRequestContent<WeatherStatus::Request>();
                if(req)
                {
                    mafMsg("Receiver request from client: " << req->client_name());
                }
                else
                {
                    mafMsg("Get request with no content, operationID=  " << msg->getRequestKeeper()->getOperationID());
                }
            }
            break;

            default:
                break;

            }

        });


        run(detached ? LaunchMode::Async : LaunchMode::AttachToCurrentThread);
    }
    void stopTest()
    {
        _stub.reset();
        this->stop();
    }

    void onEntry() override
    {
        mafMsg("Component is starting");
        _stub = Stub::createStub(ServiceID);
        mafMsg("Stub ready as well");
    }
};

}
}
