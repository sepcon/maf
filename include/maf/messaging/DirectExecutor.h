#pragma once

#include <maf/logging/Logger.h>

#include "ExecutorIF.h"

namespace maf {
namespace messaging {

class DirectExecutor : public ExecutorIF {
 public:
  virtual bool execute(CallbackType callback) noexcept override {
    if (callback) {
      try {
        callback();
        return true;
      } catch (const std::exception& e) {
        MAF_LOGGER_ERROR("Exception ocurred when executing callback: ",
                         e.what());
      }
    }
    return false;
  }
};

inline std::shared_ptr<ExecutorIF> directExecutor() {
  static std::shared_ptr<ExecutorIF> executor{new DirectExecutor{}};
  return executor;
}

}  // namespace messaging
}  // namespace maf
