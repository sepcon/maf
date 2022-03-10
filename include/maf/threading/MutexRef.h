#pragma once

namespace maf {
namespace threading {

template <class _Mutex>
class MutexReference : public std::reference_wrapper<_Mutex> {
  using _Base = std::reference_wrapper<_Mutex>;

 public:
  using _Base::_Base;
  void lock() { this->get().lock(); }
  void unlock() { this->get().unlock(); }
  bool try_lock() { return this->get().try_lock(); }
};

}  // namespace threading
}  // namespace maf
