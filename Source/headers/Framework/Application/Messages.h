#ifndef MESSAGES_H
#define MESSAGES_H

#include "headers/Framework/Messaging/Message.h"
#include <functional>

namespace thaf {
namespace app {

struct InternalMessage : public messaging::MessageBase {};
struct StartupMessage : public InternalMessage{};
struct ShutdownMessage : public InternalMessage{};
struct TimeoutMessage : public InternalMessage
{
#ifdef MESSAGIN_BY_PRIORITY
    TimeoutMessage() { setPriority(1000); }
#endif
    std::function<void()> callback;
    unsigned int timerID;
};
}
}
#endif // MESSAGES_H
