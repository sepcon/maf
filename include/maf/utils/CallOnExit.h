#pragma once
#include <functional>

namespace maf {
namespace util {

class CallOnExit {
 public:
  using CallbackType = std::function<void()>;
  template <typename Callable>
  CallOnExit(Callable&& f) : f_{std::forward<Callable>(f)} {}

  ~CallOnExit() {
    if (f_) {
      f_();
    }
  }

 private:
  CallbackType f_;
};

}  // namespace util
}  // namespace maf
