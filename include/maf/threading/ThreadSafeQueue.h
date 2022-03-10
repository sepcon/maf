#pragma once

#include <atomic>
#include <condition_variable>
#include <functional>
#include <queue>

#include "Lockable.h"

namespace maf {
namespace threading {

template <class QueueClass>
class ThreadSafeQueue {
 public:
  using reference = typename QueueClass::reference;
  using const_reference = typename QueueClass::const_reference;
  using value_type = typename QueueClass::value_type;
  using ApplyAction = std::function<void(value_type &)>;

  ThreadSafeQueue() : closed_(false) {}
  ~ThreadSafeQueue() { close(); }
  bool empty() { return queue_.atomic()->empty(); }
  void push(const value_type &data) {
    if (!isClosed()) {
      std::lock_guard lock(queue_);
      queue_->push(data);
      queueNotEmpty_.notify_one();
    }
  }
  void push(value_type &&data) {
    if (!isClosed()) {
      std::lock_guard lock(queue_);
      queue_->emplace(std::move(data));
      queueNotEmpty_.notify_one();
    }
  }

  template <
      class _Sequence,
      std::enable_if_t<
          std::is_same_v<
              std::void_t<decltype(std::declval<_Sequence>().begin())>, void>,
          bool> = true>
  void push(const _Sequence &seq) {
    if (!isClosed()) {
      std::lock_guard lock(queue_);
      for (auto &item : seq) {
        queue_->push(item);
      }
      queueNotEmpty_.notify_all();
    }
  }
  template <class TimePoint>
  bool waitUntil(value_type &value, const TimePoint &absTime) {
    std::unique_lock lock(queue_);
    if (!queueNotEmpty_.wait_until(
            lock, absTime, [this] { return !queue_->empty() || isClosed(); })) {
      return false;
    } else if (!isClosed()) {
      value = std::move(queue_->front());
      queue_->pop();
      return true;
    }
    return false;
  }

  template <class Duration>
  bool waitFor(value_type &value, const Duration &interval) {
    std::unique_lock lock(queue_);
    if (!queueNotEmpty_.wait_for(lock, interval, [this] {
          return !queue_->empty() || isClosed();
        })) {
      return false;
    } else if (!isClosed()) {
      value = std::move(queue_->front());
      queue_->pop();
      return true;
    }
    return false;
  }

  bool waitFor(value_type &value, long long ms) {
    return waitFor(value, std::chrono::milliseconds{ms});
  }

  bool wait(value_type &value) {
    std::unique_lock lock(queue_);
    queueNotEmpty_.wait(lock,
                        [this] { return !queue_->empty() || isClosed(); });
    if (!isClosed()) {
      value = std::move(queue_->front());
      queue_->pop();
      return true;
    }
    return false;
  }

  bool tryPop(value_type &value) {
    std::lock_guard lock(queue_);
    if (!queue_->empty() && !isClosed()) {
      value = queue_->front();
      queue_->pop();
      return true;
    }
    return false;
  }

  void reOpen() { closed_.store(false, std::memory_order_release); }

  void close() {
    bool alreadyClosed = false;
    closed_.compare_exchange_strong(alreadyClosed, true);
    if (!alreadyClosed) {
      queueNotEmpty_.notify_all();
    }
  }

  bool isClosed() const { return closed_.load(std::memory_order_acquire); }

  void clear(ApplyAction onClearCallback = nullptr) {
    using namespace std;
    lock_guard lock(queue_);
    if (onClearCallback) {
      while (!queue_->empty()) {
        auto v = std::move(queue_->front());
        onClearCallback(v);
        queue_->pop();
      }
    } else {
      *queue_ = {};
    }
  }

  size_t size() { return queue_.atomic()->size(); }

 private:
  Lockable<QueueClass> queue_;
  std::condition_variable_any queueNotEmpty_;
  std::atomic_bool closed_;
};

}  // namespace threading

namespace stdwrap {
template <typename T>
using Queue = std::queue<T>;

template <typename T, typename Comp>
class PriorityQueue : public std::priority_queue<T, std::vector<T>, Comp> {
 public:
  using __Base = std::priority_queue<T, std::vector<T>, Comp>;
  using std::priority_queue<T, std::vector<T>, Comp>::priority_queue;
  using reference = typename __Base::reference;
  using const_reference = typename __Base::const_reference;
  using value_type = typename __Base::value_type;

  const_reference front() const { return __Base::top(); }
};
}  // namespace stdwrap
}  // namespace maf
