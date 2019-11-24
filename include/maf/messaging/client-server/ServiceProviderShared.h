#pragma once


#include "CSStatus.h"
#include "CSMessage.h"


namespace maf {
namespace messaging {

class RequestInterface;

using AbortRequestCallback   = std::function<void(void)>;
using CSMessageContentHandlerCallback = std::function<void(const CSMsgContentBasePtr&)>;
using RequestHandlerFunction = std::function<void(const std::shared_ptr<RequestInterface>&)>;


}
}
