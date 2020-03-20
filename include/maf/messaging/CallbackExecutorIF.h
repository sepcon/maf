#ifndef CALLBACKEXECUTORINTERFACE_H
#define CALLBACKEXECUTORINTERFACE_H

#include <functional>

namespace maf {
namespace messaging {

struct CallbackExecutorIF {
  using CallbackType = std::function<void(void)>;
  virtual ~CallbackExecutorIF() = default;
  virtual bool execute(CallbackType) noexcept = 0;
};

} // namespace messaging
} // namespace maf
#endif // CALLBACKEXECUTORINTERFACE_H
