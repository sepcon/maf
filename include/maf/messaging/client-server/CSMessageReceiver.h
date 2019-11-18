#pragma once

#include "CSMessage.h"

namespace maf {
namespace messaging {

class CSMessageReceiver
{
public:
    virtual bool onIncomingMessage(const CSMessagePtr& csMsg) = 0;
};

}
}
