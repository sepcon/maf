#ifndef MESSAGES_H
#define MESSAGES_H

#include "MessageBase.h"
#include <functional>

namespace maf {
namespace messaging {

struct InternalMessage : public messaging::MessageBase {};
struct ExternalMessage : public messaging::MessageBase {};
struct StartupMessage : public InternalMessage{};
struct ShutdownMessage : public InternalMessage{};
struct CallbackExcMsg : public InternalMessage
{
    CallbackExcMsg(std::function<void()> callback_ = nullptr) : callback(std::move(callback_)) {}
    void execute() { if(callback) callback(); }
    std::function<void()> callback;
};
struct TimeoutMessage : public CallbackExcMsg
{
    TimeoutMessage() { setPriority(1000); }
    unsigned int timerID;
};
}
}
#endif // MESSAGES_H
