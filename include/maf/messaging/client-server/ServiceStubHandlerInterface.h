#pragma once

#include "RequestInterface.h"

namespace maf {
namespace messaging {

class ServiceStubHandlerInterface
{
public:
    // Intended to not having destructor
    // virtual ~ServiceStubHandlerInterface() = 0;
    virtual void onClientRequest(const std::shared_ptr<RequestInterface>& request) = 0;
    virtual void onClientAbortRequest(AbortRequestCallback callback) = 0;
};

}
}
