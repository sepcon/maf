#pragma once

#include <functional>
#include <memory>

namespace maf {
namespace messaging {

class ExecutorIF {
public:
  using CallbackType = std::function<void(void)>;
  virtual ~ExecutorIF() = default;
  virtual bool execute(CallbackType) noexcept = 0;
};

using ExecutorPtr = std::shared_ptr<ExecutorIF>;

} // namespace messaging
} // namespace maf
