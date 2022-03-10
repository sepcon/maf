#pragma once

#include <maf/patterns/Patterns.h>

#include <mutex>

namespace maf {
namespace threading {

template <class Data_, class Mutex = std::mutex>
class AtomicObject : public pattern::UnCopyable {
  template <class Pointed_>
  struct AtomicRefBase_ : public pattern::Unasignable {
   public:
    AtomicRefBase_(Pointed_ *p) : pointed_{p} { pointed_->mutex_.lock(); }

    AtomicRefBase_(AtomicRefBase_ &&other) : pointed_{other.pointed_} {
      other.pointed_ = nullptr;
    }

    AtomicRefBase_ &operator=(AtomicRefBase_ &&other) {
      pointed_ = other.pointed_;
      other.pointed_ = nullptr;
    }

    ~AtomicRefBase_() {
      if (pointed_) {
        pointed_->mutex_.unlock();
      }
    }

   protected:
    Pointed_ *pointed_ = nullptr;
  };

  template <class _D, class _M, typename = void>
  class AtomicRef_ : public AtomicRefBase_<AtomicObject<_D, _M>> {
    using Base_ = AtomicRefBase_<AtomicObject<_D, _M>>;

   public:
    using Base_::Base_;
    decltype(auto) operator->() {
      return std::addressof(this->pointed_->data_);
    }
    decltype(auto) operator*() { return this->pointed_->data_; }
  };

  template <class _D>
  static constexpr bool __isPtrRef =
      std::is_pointer_v<decltype(std::declval<_D>().get())>;

  template <class _D, class _M>
  class AtomicRef_<
      _D, _M,
      std::enable_if_t<std::is_pointer_v<decltype(std::declval<_D>().get())>,
                       void>> : public AtomicRefBase_<AtomicObject<_D, _M>> {
    using Base_ = AtomicRefBase_<AtomicObject<_D, _M>>;

   public:
    using Base_::Base_;
    decltype(auto) operator->() { return this->pointed_->data_.get(); }
    decltype(auto) operator*() { return *(this->pointed_->data_.get()); }
  };

  template <class _D, class _M, typename = void>
  struct CAtomicRef_ : public AtomicRefBase_<const AtomicObject<_D, _M>> {
    using Base_ = AtomicRefBase_<const AtomicObject<_D, _M>>;

   public:
    using Base_::Base_;
    decltype(auto) operator->() const {
      return std::addressof(Base_::pointed_->data_);
    }
    decltype(auto) operator*() const { return this->pointed_->data_; }
  };

  template <class _D, class _M>
  class CAtomicRef_<
      _D, _M,
      std::enable_if_t<std::is_pointer_v<decltype(std::declval<_D>().get())>,
                       void>> : public AtomicRefBase_<AtomicObject<_D, _M>> {
    using Base_ = AtomicRefBase_<AtomicObject<_D, _M>>;

   public:
    using Base_::Base_;
    decltype(auto) operator->() const { return this->pointed_->data_.get(); }
    decltype(auto) operator*() const { return *(this->pointed_->data_.get()); }
  };

 public:
  using DataType = Data_;
  using RefType = AtomicRef_<Data_, Mutex>;
  using CRefType = CAtomicRef_<Data_, Mutex>;

  AtomicObject() = default;
  AtomicObject(DataType p) : data_(std::move(p)) {}
  AtomicObject(Mutex mut) : mutex_(std::move(mut)) {}
  AtomicObject(DataType p, Mutex mut)
      : data_(std::move(p)), mutex_(std::move(mut)) {}

  Mutex &getMutex() { return mutex_; }
  const Mutex &getMutex() const { return mutex_; }

  DataType &lockee() { return data_; }
  const DataType &lockee() const { return data_; }

  RefType operator->() { return RefType{this}; }
  RefType operator*() { return RefType{this}; }

  CRefType operator->() const { return CRefType{this}; }
  CRefType operator*() const { return CRefType{this}; }

  RefType atomic() { return RefType{this}; }
  CRefType atomic() const { return CRefType{this}; }

  template <class _SomeType,
            std::enable_if_t<std::is_constructible_v<DataType, _SomeType>,
                             bool> = true>
  AtomicObject &operator=(_SomeType &&d) {
    auto lock = atomic();
    data_ = std::forward<_SomeType>(d);
    return *this;
  }

 private:
  DataType data_;
  mutable Mutex mutex_;
};

}  // namespace threading
}  // namespace maf
