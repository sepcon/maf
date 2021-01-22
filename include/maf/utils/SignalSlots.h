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
using SlotInvokerPtr = util::ExecutorIFPtr;

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

  void notify(ConstRef_<Args_>... args) const {
    if (valid()) {
      mySlot()->operator()(args...);
    }
  }

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

template <class... Args_>
class MultiSlotKeeper {
 public:
  using SlotPtrType = SlotPtr_<Args_...>;
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

  void notify(ConstRef_<Args_>... args) const {
    if (valid()) {
      auto clonedSlots = slots_;  // slot might disconnect
      for (const auto& s : clonedSlots) {
        (*s)(args...);
      }
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

struct DummyMutex_ {
  void lock() {}
  void unlock() {}
  bool try_lock() { return true; }
};

template <class T>
using NonAtomicObject_ = AtomicObject<T, DummyMutex_>;

template <class SlotKeeperType_>
decltype(auto) getSlots(const NonAtomicObject_<SlotKeeperType_>& skeeper) {
  return skeeper->getSlots();
}

template <class SlotKeeperType_>
decltype(auto) getSlots(const AtomicObject<SlotKeeperType_>& skeeper) {
  return skeeper->cloneSlots();
}

class ConnectableIF {
 public:
  using ConnectorPtr = shared_ptr<void>;
  using ConnectorRef = weak_ptr<void>;

  virtual bool disconnect(const ConnectorPtr& ctor) = 0;
  virtual bool connect(const ConnectorPtr& ctor) = 0;
  virtual bool connectedTo(const ConnectorPtr& ctor) = 0;
};

class Connection {
 public:
  Connection() = default;
  Connection(Connection&& other) noexcept
      : ctor_(move(other.ctor_)), hubref_{move(other.hubref_)} {}

  Connection& operator=(Connection&& other) noexcept {
    if (&other != this) {
      this->ctor_ = move(other.ctor_);
      this->hubref_ = move(other.hubref_);
      other.ctor_ = {};
    }
    return *this;
  }

  Connection(const Connection&) = delete;
  Connection& operator=(const Connection&) = delete;

  bool disconnect() noexcept {
    try {
      if (auto s = get<ConnectorRef_>(ctor_).lock()) {
        if (auto hub = hubref_.lock()) {
          if (hub->disconnect(s)) {
            ctor_ = move(s);
            return true;
          }
        } else {
          ctor_ = {};
        }
      }
    } catch (...) {
    }
    return false;
  }

  bool connected() const {
    try {
      if (auto ctor = get<ConnectorRef_>(ctor_).lock()) {
        if (auto hub = hubref_.lock()) {
          return hub->connectedTo(ctor);
        }
      }
    } catch (...) {
    }
    return false;
  }

  bool reconnect() {
    try {
      if (auto ctor = get<ConnectorPtr_>(ctor_)) {
        if (auto hub = hubref_.lock()) {
          if (hub->connect(ctor)) {
            ctor_ = ConnectorRef_{ctor};
            return true;
          } else {
            ctor_ = {};
          }
        }
      }
    } catch (...) {
    }
    return false;
  }

 private:
  using ConnectorRef_ = ConnectableIF::ConnectorRef;
  using ConnectorPtr_ = ConnectableIF::ConnectorPtr;
  using HubRef_ = weak_ptr<ConnectableIF>;

  template <class SlotsKeeper_, class... Args_>
  friend class BasicSignal_;

  Connection(const HubRef_& hub, ConnectorRef_&& s)
      : ctor_(move(s)), hubref_(hub) {}

  variant<ConnectorRef_, ConnectorPtr_> ctor_;
  HubRef_ hubref_;
};

class ScopedConnection {
 public:
  using ConnectionType = Connection;
  ScopedConnection() = default;
  ScopedConnection(ConnectionType&& c) : c_{move(c)} {}
  ScopedConnection(ScopedConnection&&) = default;
  ScopedConnection& operator=(ScopedConnection&&) = default;
  ~ScopedConnection() { c_.disconnect(); }

 private:
  ConnectionType c_;
};

class ScopedConnectionGroup : public vector<ScopedConnection> {
 public:
  using ScopedConnectionType = ScopedConnection;
  ScopedConnectionGroup& operator<<(ScopedConnectionType&& conn) {
    push_back(move(conn));
    return *this;
  }
};

template <class SlotsKeeper_, class... Args_>
class BasicSignal_ {
 public:
  using Connection = details::Connection;
  using SelfType = BasicSignal_<SlotsKeeper_, Args_...>;
  using ScopedConnectionType = ScopedConnection;
  using ScopedConnectionGroupType = ScopedConnectionGroup;
  using TrackableObjPtrType = shared_ptr<void>;
  using SlotType = Slot_<ConstRef_<Args_>...>;
  using SlotPtrType = SlotPtr_<ConstRef_<Args_>...>;
  using ConnectionPtrType = shared_ptr<Connection>;
  using ConnectionAwareSlotType = Slot_<ConnectionPtrType, Args_...>;

  BasicSignal_() = default;
  BasicSignal_(const BasicSignal_&) = default;
  BasicSignal_& operator=(const BasicSignal_&) = default;
  BasicSignal_(BasicSignal_&&) = default;
  BasicSignal_& operator=(BasicSignal_&&) = default;
  ~BasicSignal_() { disconnect(); }

  void operator()(ConstRef_<Args_>... args) const { notify(args...); }

  void notify(ConstRef_<Args_>... args) const { keeper()->notify(args...); }

  void operator()(ConstRef_<Args_>... args) { notify(args...); }

  void notify(ConstRef_<Args_>... args) { keeper()->notify(args...); }

  Connection connect(SlotType s) {
    assert(s);
    return saveSlot(makeSlotPtr(move(s)));
  }

  Connection connect(SlotType s, SlotInvokerPtr executor) {
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

  Connection connect(ConnectionAwareSlotType cas, SlotInvokerPtr executor) {
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
                     SlotInvokerPtr executor) {
    assert(executor && "executor must not be null");
    return connect(move(obj),
                   combineSlotWithExecutor<Args_...>(move(s), move(executor)));
  }

  template <
      class SlotLike,
      std::enable_if_t<is_constructible_v<SlotType, SlotLike>, bool> = true>
  Connection connect(shared_ptr<SlotLike> ps) {
    assert(ps);
    return connect(ps, ref(*ps));
  }

  Connection connect(SlotPtrType ps) {
    assert(ps && *ps);
    return saveSlot(ps);
  }

  Connection connect(SlotPtrType ps, SlotInvokerPtr executor) {
    return connect(combineSlotWithExecutor<Args_...>(move(ps), move(executor)));
  }

  template <class _Keeper, class... _Args,
            enable_if_t<is_constructible_v<SlotType,
                                           function<void(ConstRef_<_Args>...)>>,
                        bool> = true>
  Connection connect(const BasicSignal_<_Keeper, _Args...>& otherSignal) {
    using OtherSignal = BasicSignal_<_Keeper, _Args...>;
    using OtherKeeperType = typename OtherSignal::SlotsKeeperType;
    auto otherKeeperRef = weak_ptr<OtherKeeperType>{otherSignal.sharedKeeper()};

    return connect([otherKeeperRef](const ConnectionPtrType& con,
                                    ConstRef_<_Args>... args) {
      if (auto otherSharedKeeper = otherKeeperRef.lock()) {
        (*otherSharedKeeper)->notify(args...);
      } else {
        con->disconnect();
      }
    });
  }

  bool disconnect(const SlotPtrType& ps) { return keeper()->remove(ps); }

  void disconnect() { keeper()->clear(); }

  bool connected() const { return keeper()->valid(); }

 protected:
  template <class OtherKeeper_, class... OtherArgs>
  friend class BasicSignal_;

  struct MyKeeper_ : public SlotsKeeper_, public ConnectableIF {
    using Base_ = SlotsKeeper_;
    using Base_::Base_;

    bool connect(const ConnectableIF::ConnectorPtr& ctor) override {
      assert(ctor);
      return (*this)->addUnique(reinterpret_pointer_cast<SlotType>(ctor));
    }

    bool disconnect(const ConnectableIF::ConnectorPtr& ctor) override {
      return (*this)->remove(reinterpret_pointer_cast<SlotType>(ctor));
    }

    bool connectedTo(const ConnectorPtr& ctor) override {
      return (*this)->has(reinterpret_pointer_cast<SlotType>(ctor));
    }
  };

  using SlotsKeeperType = MyKeeper_;
  using SlotKeeperPtr = shared_ptr<MyKeeper_>;

  template <class... Ts>
  static decltype(auto) combineSlotWithExecutor(Slot_<Ts...>&& s,
                                                SlotInvokerPtr&& executor) {
    return combineSlotWithExecutor<Ts...>(make_shared<Slot_<Ts...>>(move(s)),
                                          move(executor));
  }

  template <class... Ts>
  static decltype(auto) combineSlotWithExecutor(shared_ptr<Slot_<Ts...>>&& ps,
                                                SlotInvokerPtr&& executor) {
    return [ps = move(ps),
            executor(move(executor))](ConstRef_<Ts>... args) mutable {
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
    auto ctor = reinterpret_pointer_cast<void>(ps);
    if (keeper()->add(move(ps))) {
      return {sharedKeeper(), move(ctor)};
    }
    return {};
  }

  SlotKeeperPtr sharedKeeper() const { return slotKeeper_; }
  const SlotsKeeper_& keeper() const { return *slotKeeper_; }
  SlotsKeeper_& keeper() { return *slotKeeper_; }

  SlotKeeperPtr slotKeeper_ = make_shared<MyKeeper_>();
};

template <class SlotsKeeper_, class... Args_>
struct BasicSignal;

template <class SlotsKeeper_, class... Args_>
struct BasicSignal : public BasicSignal_<SlotsKeeper_, Args_...> {
  using Base_ = BasicSignal_<SlotsKeeper_, Args_...>;
  using Base_::Base_;
  using State = tuple<PurgeType_<Args_>...>;
  using Base_::connect;
  Connection connect(function<void(const State&)> tpSl) {
    assert(tpSl);
    return connect(
        [tpSl{move(tpSl)}](ConstRef_<Args_>... args) { tpSl(tie(args...)); });
  }

  Connection connect(function<void(const State&)> tpSl,
                     util::ExecutorIFPtr executor) {
    assert(tpSl);
    return connect(
        [tpSl{move(tpSl)}](ConstRef_<Args_>... args) { tpSl(tie(args...)); },
        move(executor));
  }
};

template <class SlotsKeeper_, class Arg_>
struct BasicSignal<SlotsKeeper_, Arg_>
    : public BasicSignal_<SlotsKeeper_, Arg_> {
  using State = PurgeType_<Arg_>;
};

template <class... Args>
using SignalST =
    BasicSignal<NonAtomicObject_<MultiSlotKeeper<Args...>>, Args...>;

template <class... Args>
using SCSignalST =
    BasicSignal<NonAtomicObject_<SingleSlotKeeper<Args...>>, Args...>;

template <class... Args>
using SCSignal =
    BasicSignal<AtomicObject<SingleSlotKeeper<Args...>, recursive_mutex>,
                Args...>;

template <class... Args>
using Signal =
    BasicSignal<AtomicObject<MultiSlotKeeper<Args...>, recursive_mutex>,
                Args...>;

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

using details::Connection;
using details::FutureInvocation;
using details::ScopedConnection;
using details::ScopedConnectionGroup;
using details::SCSignal;
using details::SCSignalST;
using details::Signal;
using details::SignalST;
using details::SlotInvokerPtr;
using details::waitableConnect;

}  // namespace signal_slots
}  // namespace maf
