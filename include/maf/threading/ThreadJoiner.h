#pragma once

#include <thread>

namespace maf {
namespace threading {

template <class ThreadContainer> class ThreadJoiner {
  ThreadContainer &_threads;

public:
  ThreadJoiner(ThreadContainer &threads) : _threads(threads) {}
  void join() {
    for (auto &th : _threads) {
      if (th.joinable()) {
        th.join();
      }
    }
  }
  ~ThreadJoiner() { join(); }
};

template <> class ThreadJoiner<std::thread> {
  std::thread &_th;

public:
  ThreadJoiner(std::thread &th) : _th(th) {}
  ~ThreadJoiner() {
    if (_th.joinable())
      _th.join();
  }
};

} // namespace threading
} // namespace maf
