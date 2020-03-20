#ifndef SYNCCALLBACKEXECUTOR_H
#define SYNCCALLBACKEXECUTOR_H

#include "CallbackExecutorIF.h"
#include <maf/logging/Logger.h>
#include <memory>

namespace maf {
namespace messaging {

class SyncCallbackExecutor : public CallbackExecutorIF {
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
  return std::shared_ptr<SyncCallbackExecutor>(new SyncCallbackExecutor{});
}


} // namespace messaging
} // namespace maf

#endif // SYNCCALLBACKEXECUTOR_H
