#ifndef MESSAGEHANDLER_H
#define MESSAGEHANDLER_H

#include <memory>
#include <functional>
#include "MessageBase.h"

namespace maf {
namespace messaging {

using MessageHandlerFunc = std::function<void(messaging::CMessageBasePtr)>;
using SignalMsgHandlerFunc = std::function<void()>;
class MessageHandler
{
public:
    virtual void onMessage(messaging::CMessageBasePtr msg) = 0;
    virtual ~MessageHandler(){}
};

}
}
#endif // MESSAGEHANDLER_H
