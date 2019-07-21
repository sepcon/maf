#pragma once

#include "CSMessage.h"

namespace thaf {
namespace messaging {

class CSMessageReceiver
{
public:
    virtual ~CSMessageReceiver() = default;
    virtual bool onIncomingMessage(const CSMessagePtr& csMsg) = 0;
};

}
}
