#pragma once

#include "RequestKeeper.h"

namespace thaf {
namespace messaging {

class ServiceStubHandlerInterface
{
public:
    virtual ~ServiceStubHandlerInterface() = default;
    virtual void onClientRequest(const std::shared_ptr<RequestKeeperBase>& requestKeeper) = 0;

};

}
}
