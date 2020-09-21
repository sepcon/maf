#pragma once

#include <maf/logging/Logger.h>
#include <maf/utils/ExecutorIF.h>

namespace maf {
namespace util {

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

inline ExecutorIFPtr directExecutor() {
  return ExecutorIFPtr{new DirectExecutor{}};
}

}  // namespace util
}  // namespace maf
