#pragma once

#include <mutex>

namespace maf {
namespace threading {

template <class _Mutex = std::mutex>
struct Locker {
  template <class _Lockee, class _TheMutex>
  struct __Locker : public std::lock_guard<_TheMutex> {
    using _Base = std::lock_guard<_TheMutex>;
    __Locker(_Lockee& lockee, _TheMutex& m) : _Base(m), beingLocked_(&lockee) {}
    _Lockee* operator->() { return beingLocked_; }
    const _Lockee* operator->() const { return beingLocked_; }
    _Lockee& operator*() { return *beingLocked_; }
    const _Lockee& operator*() const { return *beingLocked_; }
    _Lockee* beingLocked_;
  };

  template <class _Lockee>
  decltype(auto) operator()(_Lockee& tobeLocked) {
    return __Locker<_Lockee, _Mutex>(tobeLocked, mutex_);
  }

  Locker() = default;
  Locker(_Mutex mt) : mutex_(std::move(mt)) {}
  void lock() { return mutex_.lock(); }
  void unlock() { return mutex_.unlock(); }
  bool try_lock() { return mutex_.try_lock(); }
  _Mutex mutex_;
};

}  // namespace threading
}  // namespace maf
