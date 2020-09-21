#pragma once

#include <maf/threading/AtomicObject.h>
#include <maf/utils/ExecutorIF.h>

#include <algorithm>
#include <array>
#include <memory>
#include <vector>

namespace maf {
namespace util {
namespace details {

using namespace std;
using threading::AtomicObject;

template <class... Args_>
using Slot_ = function<void(const remove_reference_t<remove_cv_t<Args_>>&...)>;

template <class... Args_>
using SlotPtr_ = shared_ptr<Slot_<Args_...>>;

template <class... Args_>
class SingleSlotKeeper {
 public:
  using SlotPtrType = SlotPtr_<Args_...>;
  using SlotsType = array<SlotPtrType, 1>;

  bool add(SlotPtrType s) {
    if (!mySlot()) {
      mySlot() = move(s);
      return true;
    }
    return false;
  }

  bool addUnique(SlotPtrType s) { return add(move(s)); }

  bool remove(const SlotPtrType& s) {
    if (has(s)) {
      clear();
      return true;
    }
    return false;
  }

  void operator()(Args_... args) const { mySlot()->operator()(move(args)...); }

  bool valid() const { return mySlot() && mySlot()->operator bool(); }
  void clear() { mySlot().reset(); }
  bool has(const SlotPtrType& s) const { return s == mySlot(); }

  SlotsType cloneSlots() const { return slots_; }
  const SlotsType& getSlots() const { return slots_; }

 private:
  SlotPtrType& mySlot() { return slots_.front(); }
  const SlotPtrType& mySlot() const { return slots_.front(); }

  SlotsType slots_;
};

template <class... Args>
class MultiSlotKeeper {
 public:
  using SlotPtrType = SlotPtr_<Args...>;
  using SlotsType = vector<SlotPtrType>;

  bool add(SlotPtrType s) {
    slots_.push_back(move(s));
    return true;
  }

  bool addUnique(SlotPtrType s) {
    if (!has(s)) {
      slots_.push_back(move(s));
      return true;
    }
    return false;
  }

  void operator()(Args... args) const {
    for (const auto& s : slots_) {
      (*s)(args...);
    }
  }

  bool remove(const SlotPtrType& s) {
    if (auto i = find(begin(slots_), end(slots_), s); i != end(slots_)) {
      slots_.erase(i);
      return true;
    }
    return false;
  }

  bool valid() const { return !slots_.empty(); }

  void clear() { slots_.clear(); }

  bool has(const SlotPtrType& s) const {
    return find(begin(slots_), end(slots_), s) != end(slots_);
  }

  SlotsType cloneSlots() const { return slots_; }
  const SlotsType& getSlots() const { return slots_; }

 protected:
  SlotsType slots_;
};

template <class T>
class NonAtomicObject_ {
 public:
  const T* operator->() const { return &d_; }
  T* operator->() { return &d_; }
  const T& operator*() const { return d_; }
  T& operator*() { return d_; }

 private:
  T d_;
};

template <class SlotKeeperType_>
decltype(auto) getSlots(const NonAtomicObject_<SlotKeeperType_>& skeeper) {
  return skeeper->getSlots();
}

template <class SlotKeeperType_>
decltype(auto) getSlots(const AtomicObject<SlotKeeperType_>& skeeper) {
  return skeeper->cloneSlots();
}

template <class SlotsKeeper_, class... Args_>
class BasicSignal_ {
  class ConnectionAwareSlotWrap_;

 public:
  class Connection;
  using ConnectionPtr = shared_ptr<Connection>;
  using SlotType = Slot_<Args_...>;
  using ConnectionAwareSlotType = Slot_<ConnectionPtr, Args_...>;
  using SlotPtrType = SlotPtr_<Args_...>;

  BasicSignal_() = default;
  BasicSignal_(const BasicSignal_&) = delete;
  BasicSignal_& operator=(const BasicSignal_&) = delete;
  BasicSignal_(BasicSignal_&&) = default;
  BasicSignal_& operator=(BasicSignal_&&) = default;

  void operator()(Args_... args) const { notify(move(args)...); }

  void notify(Args_... args) const {
    if (keeper()->valid()) {
      for (auto& s : getSlots(keeper())) {
        (*s)(args...);
      }
    }
  }

  Connection connect(SlotType s) { return saveSlot(makeSlotPtr(move(s))); }

  Connection connect(SlotType s, ExecutorIFPtr executor) {
    return saveSlot(makeSlotPtr(
        combineSlotWithExecutor<Args_...>(move(s), move(executor))));
  }

  bool connect(ConnectionAwareSlotType cas) {
    auto ps = makeSlotPtr({});
    *ps = ConnectionAwareSlotWrap_(
        move(cas), ConnectionPtr{new Connection{sharedKeeper(), ps}});
    return keeper()->add(move(ps));
  }

  bool connect(ConnectionAwareSlotType cas, ExecutorIFPtr executor) {
    auto ps = makeSlotPtr({});
    *ps = ConnectionAwareSlotWrap_(
        move(cas), move(executor),
        ConnectionPtr{new Connection{sharedKeeper(), ps}});
    return keeper()->add(move(ps));
  }

  void disconnect() { keeper()->clear(); }

 private:
  using SlotKeeperPtr = shared_ptr<SlotsKeeper_>;

  template <class... Ts>
  static decltype(auto) combineSlotWithExecutor(Slot_<Ts...>&& s,
                                                ExecutorIFPtr&& executor) {
    return [s = move(s), executor(move(executor))](
               const remove_reference_t<remove_cv_t<Ts>>&... args) {
      auto sparams = make_shared<tuple<remove_reference_t<remove_cv_t<Ts>>...>>(
          move(args)...);
      executor->execute(
          [s, sparams{move(sparams)}]() mutable { apply(s, move(*sparams)); });
    };
  }

  decltype(auto) makeSlotPtr(SlotType&& s) {
    return make_shared<SlotType>(move(s));
  }

  Connection saveSlot(SlotPtrType ps) {
    if (keeper()->add(ps)) {
      return {sharedKeeper(), move(ps)};
    }
    return {};
  }

  SlotKeeperPtr sharedKeeper() const { return slotKeeper_; }
  const SlotsKeeper_& keeper() const { return *slotKeeper_; }
  SlotsKeeper_& keeper() { return *slotKeeper_; }

  SlotKeeperPtr slotKeeper_ = make_shared<SlotsKeeper_>();

 public:
  class Connection {
   public:
    Connection() = default;
    Connection(Connection&& other) noexcept {
      if (&other != this) {
        this->slot_ = move(other.slot_);
        this->slotKeeperRef_ = move(other.slotKeeperRef_);
        other.slot_ = nullptr;
      }
    }
    Connection& operator=(Connection&& other) {
      if (&other != this) {
        this->slot_ = move(other.slot_);
        this->slotKeeperRef_ = move(other.slotKeeperRef_);
        other.slot_ = nullptr;
      }
      return *this;
    }

    Connection(const Connection&) = delete;
    Connection& operator=(const Connection&) = delete;

    bool disconnect() const {
      if (slot_) {
        if (auto slotKeeper = slotKeeperRef_.lock()) {
          return (*slotKeeper)->remove(slot_);
        }
      }
      return false;
    }

    bool connected() const {
      if (slot_) {
        if (auto keeper = slotKeeperRef_.lock()) {
          return (*keeper)->has(slot_);
        }
      }
      return false;
    }

    bool reconnect() const {
      if (slot_) {
        if (auto keeper = slotKeeperRef_.lock()) {
          (*keeper)->addUnique(slot_);
          return true;
        }
      }
      return false;
    }

   private:
    friend class BasicSignal_;
    friend class ConnectionAwareSlotWrap_;
    Connection(const SlotKeeperPtr& keeper, SlotPtrType s)
        : slotKeeperRef_(keeper), slot_(move(s)) {}

    weak_ptr<SlotsKeeper_> slotKeeperRef_;
    SlotPtrType slot_;
  };

  class ScopedConnection {
   public:
    ScopedConnection(Connection&& c) : c_{move(c)} {}
    ~ScopedConnection() { c_.disconnect(); }

   private:
    Connection c_;
  };

 private:
  class ConnectionAwareSlotWrap_ {
   public:
    ConnectionAwareSlotWrap_() = default;
    ConnectionAwareSlotWrap_(const ConnectionAwareSlotWrap_&) {
      assert(false &&
             "This code must never be executed, it is defined for satisfying "
             "requirement of std::function functor object only");
    }
    ConnectionAwareSlotWrap_(ConnectionAwareSlotWrap_&&) = default;
    ConnectionAwareSlotWrap_& operator=(ConnectionAwareSlotWrap_&&) = default;
    ConnectionAwareSlotWrap_(ConnectionAwareSlotType&& s, ConnectionPtr&& c)
        : slot_(move(s)), con_(move(c)) {}

    ConnectionAwareSlotWrap_(ConnectionAwareSlotType&& s,
                             ExecutorIFPtr&& executor, ConnectionPtr&& c)
        : slot_(combineSlotWithExecutor<ConnectionPtr, Args_...>(
              move(s), move(executor))),
          con_(move(c)) {}

    void operator()(
        const remove_reference_t<remove_cv_t<Args_>>&... args) const {
      slot_(con_, args...);
    }

   private:
    ConnectionAwareSlotType slot_;
    shared_ptr<Connection> con_;
  };
};

enum class MultiThread { Yes, No };

template <MultiThread ts, class SlotsKeeper_, class... Args_>
class BasicSignal;

template <class SlotsKeeper_, class... Args>
class BasicSignal<MultiThread::No, SlotsKeeper_, Args...>
    : public BasicSignal_<NonAtomicObject_<SlotsKeeper_>, Args...> {};

template <class SlotsKeeper_, class... Args>
class BasicSignal<MultiThread::Yes, SlotsKeeper_, Args...>
    : public BasicSignal_<AtomicObject<SlotsKeeper_, mutex>, Args...> {};

template <class... Args>
using SCSignalST =
    BasicSignal<MultiThread::No, SingleSlotKeeper<Args...>, Args...>;

template <class... Args>
using SignalST =
    BasicSignal<MultiThread::No, MultiSlotKeeper<Args...>, Args...>;

template <class... Args>
using SCSignal =
    BasicSignal<MultiThread::Yes, SingleSlotKeeper<Args...>, Args...>;

template <class... Args>
using Signal = BasicSignal<MultiThread::Yes, MultiSlotKeeper<Args...>, Args...>;

}  // namespace details

using details::SCSignal;
using details::SCSignalST;
using details::Signal;
using details::SignalST;

}  // namespace util
}  // namespace maf
