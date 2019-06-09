#ifndef MESSAGES_H
#define MESSAGES_H

#include "headers/Messaging/Message.h"
#include <functional>

namespace thaf {
namespace app {

struct InternalMessage : public Messaging::Message {};
struct StartupMessage : public InternalMessage{};
struct ShutdownMessage : public InternalMessage{};
struct TimeoutMessage : public InternalMessage
{
    std::function<void()> callback;
};
} }
#endif // MESSAGES_H
