#ifndef QUEUEINGCALLBACKEXECUTOR_H
#define QUEUEINGCALLBACKEXECUTOR_H

#include <maf/messaging/BasicMessages.h>
#include <maf/messaging/CallbackExecutorIF.h>
#include <maf/messaging/Component.h>

namespace maf {
namespace messaging {

class AsyncCallbackExecutor : public CallbackExecutorIF {
  using CompWPtr = std::weak_ptr<Component>;

public:
  AsyncCallbackExecutor(CompWPtr comp) : _wpcomp{std::move(comp)} {}

  bool execute(CallbackType callback) noexcept override {
    if (auto comp = _wpcomp.lock()) {
      return comp->post<CallbackExcMsg>(std::move(callback));
    } else {
      return false;
    }
  }

  CompWPtr component() const { return _wpcomp; }

private:
  CompWPtr _wpcomp;
};

inline std::shared_ptr<CallbackExecutorIF>
asyncExecutor(std::weak_ptr<Component> comp) {
  return std::shared_ptr<AsyncCallbackExecutor>(
      new AsyncCallbackExecutor{std::move(comp)});
}

} // namespace messaging
} // namespace maf
#endif // QUEUEINGCALLBACKEXECUTOR_H
