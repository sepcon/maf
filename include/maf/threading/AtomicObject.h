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

  class AtomicRef_ : public AtomicRefBase_<AtomicObject> {
    using Base_ = AtomicRefBase_<AtomicObject>;

   public:
    using Base_::Base_;
    Data_ *operator->() { return std::addressof(this->pointed_->data_); }
    Data_ &operator*() { return this->pointed_->data_; }
  };
  struct CAtomicRef_ : public AtomicRefBase_<const AtomicObject> {
    using Base_ = AtomicRefBase_<const AtomicObject>;

   public:
    using Base_::Base_;
    const Data_ *operator->() const {
      return std::addressof(Base_::pointed_->data_);
    }
    const Data_ &operator*() const { return this->pointed_->data_; }
  };

 public:
  using DataType = Data_;
  using AtomicRefType = AtomicRef_;
  using CAtomicRefType = CAtomicRef_;

  AtomicObject() = default;
  AtomicObject(const DataType &p) : data_(p) {}
  AtomicObject(DataType &&p) : data_(std::move(p)) {}

  Mutex &getMutex() { return mutex_; }
  const Mutex &getMutex() const { return mutex_; }

  DataType &lockee() { return data_; }
  const DataType &lockee() const { return data_; }

  AtomicRefType operator->() { return AtomicRefType{this}; }
  AtomicRefType operator*() { return AtomicRefType{this}; }

  CAtomicRefType operator->() const { return CAtomicRefType{this}; }
  CAtomicRefType operator*() const { return CAtomicRefType{this}; }

  AtomicRefType atomic() { return AtomicRefType{this}; }
  CAtomicRefType atomic() const { return CAtomicRefType{this}; }

 private:
  mutable Mutex mutex_;
  DataType data_;
};

}  // namespace threading
}  // namespace maf
