#ifndef MESSAGEHANDLER_H
#define MESSAGEHANDLER_H

#include <memory>
#include <functional>
#include "Message.h"

namespace thaf {
namespace app {

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
