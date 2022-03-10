#pragma once

#include <maf/patterns/Patterns.h>

#include <mutex>

namespace maf {
namespace threading {

template <class Protected, class Mutex = std::mutex>
class Lockable : public pattern::Unasignable {
  template <class Lockable_, class Protected_>
  struct AtomicRef : public pattern::Unasignable {
   public:
    AtomicRef(Lockable_ *p) : _lockable{p} { _lockable->lock(); }

    ~AtomicRef() { _lockable->unlock(); }

    Protected_ *operator->() { return _lockable->operator->(); }
    Protected_ &operator*() { return _lockable->operator*(); }

   private:
    Lockable_ *_lockable;
  };

 public:
  using DataType = Protected;
  using AtomicSession = AtomicRef<Lockable<Protected, Mutex>, Protected>;
  using CAtomicSession =
      AtomicRef<const Lockable<Protected, Mutex>, const Protected>;

  Lockable() = default;
  template <std::enable_if_t<std::is_copy_assignable_v<DataType>, bool> = true>
  Lockable(const DataType &p) : _protected(p) {}
  template <std::enable_if_t<std::is_move_assignable_v<DataType>, bool> = true>
  Lockable(DataType &&p) : _protected(std::move(p)) {}

  void lock() const { _mutex.lock(); }
  void unlock() const { _mutex.unlock(); }
  bool try_lock() const { return _mutex.try_lock(); }

  DataType *operator->() { return &_protected; }
  DataType &operator*() { return _protected; }
  const DataType *operator->() const { return &_protected; }
  const DataType &operator*() const { return _protected; }

  CAtomicSession atomic() const { return CAtomicSession{this}; }
  AtomicSession atomic() { return AtomicSession{this}; }

  Mutex &getMutex() { return _mutex; }

  template <class _SomeType,
            std::enable_if_t<std::is_constructible_v<DataType, _SomeType>,
                             bool> = true>
  Lockable &operator=(_SomeType &&d) {
    auto lock = atomic();
    _protected = std::forward<_SomeType>(d);
    return *this;
  }

 private:
  mutable Mutex _mutex;
  DataType _protected;
};

}  // namespace threading
}  // namespace maf
