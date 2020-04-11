#ifndef SYNCCALLBACKEXECUTOR_H
#define SYNCCALLBACKEXECUTOR_H

#include "CallbackExecutorIF.h"
#include <maf/logging/Logger.h>
#include <maf/patterns/Patterns.h>
#include <memory>

namespace maf {
namespace messaging {

class SyncCallbackExecutor : public CallbackExecutorIF, pattern::Unasignable {
public:
  virtual bool execute(CallbackType callback) noexcept override {
    if (callback) {
      try {
        callback();
        return true;
      } catch (const std::exception &e) {
        MAF_LOGGER_ERROR("SyncCallbackExecutor: "
                         "Error while executing callback: ",
                         e.what());
      }
    }
    return false;
  }
};

inline std::shared_ptr<CallbackExecutorIF> syncExecutor() {
  static std::shared_ptr<CallbackExecutorIF> executor{
      new SyncCallbackExecutor{}};
  return executor;
}

} // namespace messaging
} // namespace maf

#endif // SYNCCALLBACKEXECUTOR_H
