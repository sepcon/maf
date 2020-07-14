#pragma once

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
      } catch (const std::exception & /*e*/) {
      }
    }
    return false;
  }
};

inline std::shared_ptr<ExecutorIF> directExecutor() {
  static std::shared_ptr<ExecutorIF> executor{new DirectExecutor{}};
  return executor;
}

} // namespace messaging
} // namespace maf
