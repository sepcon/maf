#pragma once

#include <maf/threading/AtomicObject.h>
#include <maf/utils/ExecutorIF.h>

#include <algorithm>
#include <array>
#include <cassert>
#include <future>
#include <memory>
#include <variant>
#include <vector>

namespace maf {
namespace signal_slots {
namespace details {

using namespace std;
using threading::AtomicObject;
using util::ExecutorIFPtr;

template <class T>
using PurgeType_ = remove_reference_t<remove_cv_t<T>>;

template <class T>
using ConstRef_ = const PurgeType_<T>&;

template <class... Args_>
using Slot_ = function<void(ConstRef_<Args_>...)>;

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
  void clear() {
    if (mySlot()) {
      *mySlot() = {};
      mySlot().reset();
    }
  }
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

  void clear() {
    for (auto& s : slots_) {
      *s = {};
    }
    slots_.clear();
  }

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
 public:
  class Connection;
  using TrackableObjPtrType = shared_ptr<void>;
  using SlotType = Slot_<Args_...>;
  using SingleArgSlotType = Slot_<tuple<Args_...>>;
  using SlotPtrType = SlotPtr_<Args_...>;
  using ConnectionPtrType = shared_ptr<Connection>;
  using ConnectionAwareSlotType = Slot_<ConnectionPtrType, Args_...>;

  ~BasicSignal_() { disconnect(); }
  BasicSignal_() = default;
  BasicSignal_(const BasicSignal_&) = default;
  BasicSignal_& operator=(const BasicSignal_&) = default;
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

  Connection connect(SlotType s) {
    assert(s);
    return saveSlot(makeSlotPtr(move(s)));
  }

  Connection connect(SlotType s, ExecutorIFPtr executor) {
    assert(executor && s);
    return connect(combineSlotWithExecutor<Args_...>(move(s), move(executor)));
  }

  Connection connect(ConnectionAwareSlotType cas) {
    auto ps = makeSlotPtr({});
    *ps = [cas{move(cas)},
           conPtr = ConnectionPtrType{new Connection{sharedKeeper(), ps}}](
              ConstRef_<Args_>... args) mutable { cas(move(conPtr), args...); };
    return saveSlot(move(ps));
  }

  Connection connect(ConnectionAwareSlotType cas, ExecutorIFPtr executor) {
    return connect(combineSlotWithExecutor<ConnectionPtrType, Args_...>(
        move(cas), move(executor)));
  }

  Connection connect(TrackableObjPtrType obj, SlotType s) {
    assert(s && obj && "The tracked object and slot must not be null");
    return connect([sl{move(s)}, trackedObjRef = weak_ptr{obj}](
                       ConnectionPtrType con, Args_... args) {
      if (auto trackedObj = trackedObjRef.lock()) {
        sl(move(args)...);
      } else {
        con->disconnect();
      }
    });
  }

  Connection connect(TrackableObjPtrType obj, SlotType s,
                     ExecutorIFPtr executor) {
    assert(executor && "executor must not be null");
    return connect(move(obj),
                   combineSlotWithExecutor<ConnectionPtrType, Args_...>(
                       move(s), move(executor)));
  }

  template <
      class SlotLike,
      std::enable_if_t<is_constructible_v<SlotType, SlotLike>, bool> = true>
  Connection connect(shared_ptr<SlotLike> ps) {
    assert(ps);
    return connect(ps, ref(*ps));
  }

  //  Connection connect(SlotPtrType ps) {
  //    assert(ps && *ps);
  //    return saveSlot(ps);
  //  }

  Connection connect(SlotPtrType ps, ExecutorIFPtr executor) {
    return connect(combineSlotWithExecutor<Args_...>(move(ps), move(executor)));
  }

  bool disconnect(const SlotPtrType& ps) { return keeper()->remove(ps); }

  void disconnect() { keeper()->clear(); }

 private:
  using SlotKeeperPtr = shared_ptr<SlotsKeeper_>;

  template <class... Ts>
  static decltype(auto) combineSlotWithExecutor(Slot_<Ts...>&& s,
                                                ExecutorIFPtr&& executor) {
    return combineSlotWithExecutor<Ts...>(make_shared<Slot_<Ts...>>(move(s)),
                                          move(executor));
  }

  template <class... Ts>
  static decltype(auto) combineSlotWithExecutor(shared_ptr<Slot_<Ts...>>&& ps,
                                                ExecutorIFPtr&& executor) {
    return [ps = move(ps), executor(move(executor))](ConstRef_<Ts>... args) {
      auto sparams = make_shared<tuple<PurgeType_<Ts>...>>(move(args)...);
      executor->execute([ps, sparams{move(sparams)}]() mutable {
        apply(*ps, move(*sparams));
      });
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
    Connection(Connection&& other) noexcept
        : slotSwitch_(move(other.slotSwitch_)),
          slotKeeperRef_{move(other.slotKeeperRef_)} {
      other.slotSwitch_ = {};
    }

    Connection& operator=(Connection&& other) {
      if (&other != this) {
        this->slotSwitch_ = move(other.slotSwitch_);
        this->slotKeeperRef_ = move(other.slotKeeperRef_);
        other.slotSwitch_ = {};
      }
      return *this;
    }

    Connection(const Connection&) = delete;
    Connection& operator=(const Connection&) = delete;

    bool disconnect() {
      try {
        if (auto s = get<SlotRefType>(slotSwitch_).lock()) {
          if (auto slotKeeper = slotKeeperRef_.lock()) {
            if ((*slotKeeper)->remove(s)) {
              slotSwitch_ = move(s);
              return true;
            }
          } else {
            slotSwitch_ = {};
          }
        }
      } catch (...) {
      }
      return false;
    }

    bool connected() const {
      try {
        if (auto s = get<SlotRefType>(slotSwitch_).lock()) {
          if (auto keeper = slotKeeperRef_.lock()) {
            return (*keeper)->has(s);
          }
        }
      } catch (...) {
      }
      return false;
    }

    bool reconnect() {
      try {
        if (auto s = get<SlotPtrType>(slotSwitch_)) {
          if (auto keeper = slotKeeperRef_.lock()) {
            if ((*keeper)->addUnique(s)) {
              slotSwitch_ = SlotRefType{s};
              return true;
            } else {
              slotSwitch_ = {};
            }
          }
        }
      } catch (...) {
      }
      return false;
    }

   private:
    using SlotRefType = weak_ptr<SlotType>;
    friend class BasicSignal_;
    Connection(const SlotKeeperPtr& keeper, SlotRefType&& s)
        : slotKeeperRef_(keeper), slotSwitch_(move(s)) {}

    weak_ptr<SlotsKeeper_> slotKeeperRef_;
    variant<SlotRefType, SlotPtrType> slotSwitch_;
  };

  class ScopedConnection {
   public:
    ScopedConnection(Connection&& c) : c_{move(c)} {}
    ~ScopedConnection() { c_.disconnect(); }

   private:
    Connection c_;
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
class SCSignalST
    : public BasicSignal<MultiThread::No, SingleSlotKeeper<Args...>, Args...> {
};

template <class... Args>
class SignalST
    : public BasicSignal<MultiThread::No, MultiSlotKeeper<Args...>, Args...> {};

template <class... Args>
class SCSignal
    : public BasicSignal<MultiThread::Yes, SingleSlotKeeper<Args...>, Args...> {
};

template <class... Args>
class Signal
    : public BasicSignal<MultiThread::Yes, MultiSlotKeeper<Args...>, Args...> {
};

template <template <typename...> class Signal_, typename... Args_>
class FutureInvocation {
  using Params = tuple<PurgeType_<Args_>...>;
  using SignalType = Signal_<Args_...>;
  using ConnectionType = typename SignalType::Connection;

 public:
  using SlotType = Slot_<Args_...>;
  FutureInvocation() = default;
  FutureInvocation(ConnectionType connection, SlotType&& s, future<Params>&& f)
      : signalConnection_{move(connection)}, slot_{move(s)}, impl_{move(f)} {}

  FutureInvocation(FutureInvocation&&) = default;
  FutureInvocation& operator=(FutureInvocation&&) = default;
  FutureInvocation(const FutureInvocation&) = delete;
  FutureInvocation& operator=(const FutureInvocation&) = default;

  ~FutureInvocation() { cancel(); }

  bool valid() const { return impl_.valid(); }
  bool waitFor(const std::chrono::milliseconds& dur) {
    if (impl_.valid()) {
      try {
        if (impl_.wait_for(dur) == future_status::ready) {
          apply(slot_, impl_.get());
          return true;
        }
      } catch (const future_error&) {
      }
    }
    return false;
  }

  bool waitUtil(const chrono::system_clock::time_point& tp) {
    if (impl_.valid()) {
      try {
        if (impl_.wait_until(tp) == future_status::ready) {
          apply(slot_, impl_.get());
          return true;
        }
      } catch (const future_error&) {
      }
    }
    return false;
  }

  bool wait() {
    if (impl_.valid()) {
      try {
        apply(slot_, impl_.get());
        return true;
      } catch (const future_error&) {
      }
    }
    return false;
  }
  void cancel() { signalConnection_.disconnect(); }

 private:
  ConnectionType signalConnection_;
  SlotType slot_;
  future<Params> impl_;
};

template <template <class...> class Signal_, class... Args_>
auto waitableConnect(Signal_<Args_...>& sig,
                     typename Signal_<Args_...>::SlotType sl) {
  using SignalType = Signal_<Args_...>;
  using FutureType = FutureInvocation<Signal_, Args_...>;
  using Params = tuple<PurgeType_<Args_>...>;

  auto prom = make_shared<promise<Params>>();
  auto f = prom->get_future();
  auto connection =
      sig.connect([prom(move(prom))](typename SignalType::ConnectionPtrType con,
                                     ConstRef_<Args_>... args) {
        con->disconnect();
        prom->set_value({args...});
      });

  if (connection.connected()) {
    return FutureType{move(connection), move(sl), move(f)};
  }

  return FutureType{};
}

}  // namespace details

using details::BasicSignal;
using details::FutureInvocation;
using details::SCSignal;
using details::SCSignalST;
using details::Signal;
using details::SignalST;
using details::waitableConnect;

}  // namespace signal_slots
}  // namespace maf
