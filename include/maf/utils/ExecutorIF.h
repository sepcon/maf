#pragma once

#include <functional>
#include <memory>

namespace maf {
namespace util {

class ExecutorIF {
 public:
  using CallbackType = std::function<void(void)>;
  virtual ~ExecutorIF() = default;
  virtual bool execute(CallbackType) noexcept = 0;
};

using ExecutorIFPtr = std::shared_ptr<ExecutorIF>;

}  // namespace util
}  // namespace maf
