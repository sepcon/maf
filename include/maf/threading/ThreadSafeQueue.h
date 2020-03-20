#ifndef THREADSAFEQUEUE_H
#define THREADSAFEQUEUE_H

#include "Lockable.h"
#include <atomic>
#include <condition_variable>
#include <functional>
#include <queue>

namespace maf {
namespace threading {

template <class QueueClass> class ThreadSafeQueue {
public:
  using reference = typename QueueClass::reference;
  using const_reference = typename QueueClass::const_reference;
  using value_type = typename QueueClass::value_type;
  using ApplyAction = std::function<void(value_type &)>;

  ThreadSafeQueue() : _closed(false) {}
  ~ThreadSafeQueue() { close(); }
  bool empty() { return _queue.atomic()->empty(); }
  void push(const value_type &data) {
    if (!isClosed()) {
      std::lock_guard lock(_queue);
      _queue->push(data);
      _condVar.notify_one();
    }
  }
  void push(value_type &&data) {
    if (!isClosed()) {
      std::lock_guard lock(_queue);
      _queue->emplace(data);
      _condVar.notify_one();
    }
  }

  bool waitFor(value_type &value, long milliseconds) {
    std::unique_lock lock(_queue);
    std::cv_status waitResult = std::cv_status::no_timeout;
    if (!isClosed() && _queue->empty()) {
      if (!_condVar.wait_for(lock, std::chrono::milliseconds(milliseconds),
                             [this] { return !_queue->empty(); })) {
        return false;
      }
    }

    if (waitResult == std::cv_status::no_timeout && !isClosed()) {
      value = _queue->front();
      _queue->pop();
      return true;
    } else {
      return false;
    }
  }

  bool wait(value_type &value) {
    std::unique_lock lock(_queue);
    while (!isClosed() && _queue->empty()) {
      _condVar.wait(lock);
    }
    if (!isClosed()) {
      value = std::move(_queue->front());
      _queue->pop();
      return true;
    } else {
      return false;
    }
  }

  bool tryPop(value_type &value) {
    std::lock_guard lock(_queue);
    if (!isClosed() && !_queue->empty()) {
      value = _queue->front();
      _queue->pop();
      return true;
    } else {
      return false;
    }
  }

  void close() {
    if (!isClosed()) {
      _closed.store(true, std::memory_order_release);
      _condVar.notify_all();
    }
  }

  bool isClosed() const { return _closed.load(std::memory_order_acquire); }

  void clear(ApplyAction onClearCallback = nullptr) {
    std::lock_guard lock(_queue);
    if (onClearCallback) {
      while (!_queue->empty()) {
        auto v = _queue->front();
        onClearCallback(v);
        _queue->pop();
      }
    } else {
      while (!_queue->empty()) {
        _queue->pop();
      }
    }
  }

  size_t size() { return _queue.atomic()->size(); }

private:
  Lockable<QueueClass> _queue;
  std::condition_variable_any _condVar;
  std::atomic_bool _closed;
};

} // namespace threading

namespace stdwrap {
template <typename T> using Queue = std::queue<T>;

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
} // namespace stdwrap
} // namespace maf

#endif // THREADSAFEQUEUE_H
