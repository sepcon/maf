#ifndef MESSAGEHANDLER_H
#define MESSAGEHANDLER_H

#include <memory>
#include <functional>
#include "MessageBase.h"

namespace maf {
namespace messaging {

template <typename Message>
using MessageHandlerFunc = std::function<void(CMessagePtr<Message>&)>;
using SignalMsgHandlerFunc = std::function<void()>;
class MessageHandler
{
public:
    virtual void onMessage(CMessageBasePtr msg) = 0;
    virtual ~MessageHandler() = default;
};

}
}
#endif // MESSAGEHANDLER_H
