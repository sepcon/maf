#ifndef MESSAGES_H
#define MESSAGES_H

#include "CompMessageBase.h"
#include <maf/utils/cppextension/Invoker.h>
#include <functional>

namespace maf {
namespace messaging {

struct CallbackExcMsg : public CompMessageBase
{
    template<class Callback, class... Args>
    CallbackExcMsg(Callback&& callback_, Args&&... args) {
        auto invoker = util::makeInvoker(
            std::forward<Callback>(callback_),
            std::forward<Args>(args)...
            );
        callback = [invoker = std::move(invoker)]() mutable {
            invoker.invoke();
        };
    }
    void execute()
    {
        if(callback) callback();
    }

private:
    std::function<void()> callback;
};
struct TimeoutMessage : public CallbackExcMsg
{
    TimeoutMessage(unsigned int timerID_, std::function<void()> timeoutCallback): CallbackExcMsg(std::move(timeoutCallback)), timerID(timerID_)
    {
        setPriority(1000);
    }
    unsigned int timerID;
};
}
}
#endif // MESSAGES_H
