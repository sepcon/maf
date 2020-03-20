#ifndef MESSAGES_H
#define MESSAGES_H

#include <functional>

namespace maf {
namespace messaging {

struct CallbackExcMsg {
  template <class Callback, class... Args>
  CallbackExcMsg(Callback &&callback_, Args &&... args)
      : callback{std::bind(std::forward<Callback>(callback_),
                           std::forward<Args>(args)...)} {}

  void execute() {
    if (callback)
      callback();
  }

private:
  std::function<void()> callback;
};
} // namespace messaging
} // namespace maf
#endif // MESSAGES_H
