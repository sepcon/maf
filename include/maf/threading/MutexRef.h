#pragma once

#include <cassert>

namespace maf {
namespace threading {

template <class _Mutex>
class MutexReference {
  _Mutex* mt_ = nullptr;

 public:
  MutexReference() = default;
  MutexReference(_Mutex& m) : mt_(&m) {}
  MutexReference(MutexReference&&) = default;
  MutexReference(MutexReference const&) = default;
  MutexReference& operator=(MutexReference const&) = default;
  MutexReference& operator=(MutexReference&&) = default;

  void lock() {
    assert(mt_);
    mt_->lock();
  }
  void unlock() {
    assert(mt_);
    mt_->unlock();
  }
  bool try_lock() {
    assert(mt_);
    return mt_->try_lock();
  }
};

template <class _Mutex>
MutexReference<_Mutex> mutexRef(_Mutex& mt) {
  return MutexReference<_Mutex>(mt);
}

}  // namespace threading
}  // namespace maf
