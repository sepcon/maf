#pragma once

#include <functional>

namespace maf {
namespace util {
namespace details {

using namespace std;

template <class Output_, class Input_>
class ProcessorBase_ {
 public:
  using Output = Output_;
  using Input = Input_;
  using Callback = function<Output(Input)>;

  Output operator()(Input in) {
    if (cb_) {
      return cb_(move(in));
    } else if constexpr (!is_same_v<void, Output>) {
      return {};
    } else {
      return;
    }
  }

  operator bool() const { return valid(); }

  bool connected() const { return !cb_; }

  bool valid() const { return !connected(); }

  ProcessorBase_(Callback cb) : cb_{move(cb)} {};

 protected:
  Callback cb_;
};

template <class Output_>
class ProcessorBase_<Output_, void> {
 public:
  using Output = Output_;
  using Input = void;
  using Callback = function<Output()>;

  Output operator()() {
    if (cb_) {
      return cb_();
    } else if constexpr (is_same_v<void, Output>) {
      return {};
    } else {
      return;
    }
  }

  operator bool() const { return valid(); }

  bool empty() const { return !cb_; }

  bool valid() const { return !empty(); }

  ProcessorBase_(Callback cb) : cb_{move(cb)} {};

 protected:
  Callback cb_;
};

template <class Output_, class Input_>
class ProcessorChainBase_ : public ProcessorBase_<Output_, Input_> {
  using Base = ProcessorBase_<Output_, Input_>;

 public:
  using Base::Base;
  template <class NextCallback>
  auto then(NextCallback nextCb) {
    using NextOutput = decltype(nextCb(std::declval<Output_>()));
    return ProcessorChainBase_<NextOutput, Input_>(
        [cb{move(this->cb_)}, nextCb{move(nextCb)}](Input_ in) -> NextOutput {
          if constexpr (is_same_v<void, Output_>) {
            cb(move(in));
            return nextCb();
          } else {
            return nextCb(cb(move(in)));
          }
        });
  }
};

template <class Output_>
class ProcessorChainBase_<Output_, void>
    : public ProcessorBase_<Output_, void> {
  using Base = ProcessorBase_<Output_, void>;

 public:
  using Base::Base;
  template <class NextCallback>
  auto then(NextCallback nextCb) {
    using NextOutput = decltype(nextCb());
    return ProcessorChainBase_<NextOutput, void>(
        [cb{move(this->cb_)}, nextCb{move(nextCb)}]() -> NextOutput {
          if constexpr (is_same_v<void, Output_>) {
            cb();
            return nextCb();
          } else {
            return nextCb(cb());
          }
        });
  }
};

template <class Input_>
class ProcessorChainBase_<void, Input_> : public ProcessorBase_<void, Input_> {
  using Base = ProcessorBase_<void, Input_>;

 public:
  using Base::Base;
  template <class NextCallback>
  auto then(NextCallback nextCb) {
    using NextOutput = decltype(nextCb());
    return ProcessorChainBase_<NextOutput, Input_>(
        [cb{move(this->cb_)}, nextCb{move(nextCb)}](Input_ in) -> NextOutput {
          return nextCb(cb(move(in)));
        });
  }
};

template <class Input_>
class ProcessorChain : public ProcessorChainBase_<Input_, Input_> {
  using Base = ProcessorChainBase_<Input_, Input_>;

 public:
  ProcessorChain() : Base([](Input_ in) { return in; }){};
};

}  // namespace details

using details::ProcessorChain;

}  // namespace util
}  // namespace maf
