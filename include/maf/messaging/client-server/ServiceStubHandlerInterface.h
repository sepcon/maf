#pragma once

#include "RequestKeeper.h"

namespace maf {
namespace messaging {

class ServiceStubHandlerInterface
{
public:
    virtual ~ServiceStubHandlerInterface() = default;
    virtual void onClientRequest(const std::shared_ptr<RequestKeeperBase>& requestKeeper) = 0;
    virtual void onClientAbortRequest(RequestKeeperBase::AbortCallback callback) = 0;
};

}
}
