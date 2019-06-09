#ifndef MESSAGEHANDLER_H
#define MESSAGEHANDLER_H

#include <memory>
#include <functional>
#include "Messages.h"

namespace thaf {
namespace app {

using MessageHandlerFunc = std::function<void(Messaging::CMessagePtr)>;
class MessageHandler
{
public:
    virtual void onMessage(Messaging::CMessagePtr msg) = 0;
    virtual ~MessageHandler(){}
};

}
}
#endif // MESSAGEHANDLER_H
