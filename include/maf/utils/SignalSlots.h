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
      assert(!notifying_ && "Cyclic connection detected!");
      notifying_ = true;
      mySlot()->operator()(args...);
      notifying_ = false;
    }
  }

  bool valid() const { return mySlot() && mySlot()->operator bool(); }
  void clear() {
    if (mySlot()) {
      mySlot().reset();
    }
  }
  bool has(const SlotPtrType& s) const { return s == mySlot(); }
  size_t size() const { return mySlot() ? 1 : 0; }

  SlotPtrType lastSlot() const { return mySlot(); }

 private:
  SlotPtrType& mySlot() { return slots_.front(); }
  const SlotPtrType& mySlot() const { return slots_.front(); }

  SlotsType slots_;
  mutable bool notifying_ = false;
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
      assert(!notifying_ && "Cyclic connection detected");
      notifying_ = true;

      for (auto it = begin(slots_); it != end(slots_);) {
        if (!*it) {  // this this slot is disconnected;
          it = slots_.erase(it);
        } else {
          std::invoke(*(*it), args...);
          ++it;
        }
      }
      notifying_ = false;
    }
  }

  bool remove(const SlotPtrType& s) {
    if (!notifying_) {
      if (auto i = find(begin(slots_), end(slots_), s); i != end(slots_)) {
        slots_.erase(i);
        return true;
      }
    } else {
      if (auto i = find(begin(slots_), end(slots_), s); i != end(slots_)) {
        i->reset();
        return true;
      }
    }
    return false;
  }

  bool valid() const { return !slots_.empty(); }

  void clear() { slots_.clear(); }

  bool has(const SlotPtrType& s) const {
    return find(begin(slots_), end(slots_), s) != end(slots_);
  }

  size_t size() const { return slots_.size(); }

  SlotPtrType lastSlot() const { return slots_.back(); }

 protected:
  mutable SlotsType slots_;
  mutable bool notifying_ = false;
};

struct DummyMutex_ {
  void lock() {}
  void unlock() {}
  bool try_lock() { return true; }
};

template <class T>
using NonAtomicObject_ = AtomicObject<T, DummyMutex_>;

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
  ~Connection() {}
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

  template <class _SlotType>
  auto getSlotPtr() const {
    if (holds_alternative<ConnectorPtr_>(ctor_)) {
      return reinterpret_pointer_cast<_SlotType>(get<ConnectorPtr_>(ctor_));
    } else if (holds_alternative<ConnectorRef_>(ctor_)) {
      return reinterpret_pointer_cast<_SlotType>(
          get<ConnectorRef_>(ctor_).lock());
    } else {
      return shared_ptr<_SlotType>{};
    }
  }

 private:
  using ConnectorRef_ = ConnectableIF::ConnectorRef;
  using ConnectorPtr_ = ConnectableIF::ConnectorPtr;
  using HubRef_ = weak_ptr<ConnectableIF>;

  template <class SlotsKeeper_, class... Args_>
  friend class BasicSignal_;

  bool holdingConnectorRef() const {
    return holds_alternative<ConnectorRef_>(ctor_);
  }

  bool holdingConnector() const {
    return holds_alternative<ConnectorPtr_>(ctor_);
  }

  void clear() { ctor_ = {}; }
  Connection(const HubRef_& hub, ConnectorRef_&& s)
      : ctor_(move(s)), hubref_(hub) {}

  variant<ConnectorRef_, ConnectorPtr_> ctor_;
  HubRef_ hubref_;
};
using ConnectionPtr = shared_ptr<Connection>;

class ScopedConnection {
 public:
  using ConnectionType = Connection;
  ScopedConnection() = default;
  ScopedConnection(ConnectionType&& c) : c_{move(c)} {}
  ScopedConnection(ScopedConnection&&) = default;
  ScopedConnection& operator=(ScopedConnection&& other) {
    disconnect();
    c_ = move(other.c_);
    return *this;
  }

  ~ScopedConnection() { disconnect(); }

 private:
  void disconnect() {
    if (c_.connected()) {
      c_.disconnect();
    }
  }

  ConnectionType c_;
};

template <class SlotsKeeper_, class... Args_>
class BasicSignal_ {
 public:
  using ConnectionType = details::Connection;
  using SelfType = BasicSignal_<SlotsKeeper_, Args_...>;
  using ScopedConnectionType = ScopedConnection;
  using TrackableObjPtrType = shared_ptr<void>;
  using SlotType = Slot_<ConstRef_<Args_>...>;
  using SlotPtrType = SlotPtr_<ConstRef_<Args_>...>;
  using ConnectionPtrType = shared_ptr<ConnectionType>;
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

  ConnectionType connect(SlotType s) {
    assert(s);
    return saveSlot(makeSlotPtr(move(s)));
  }

  ConnectionType connect(SlotType s, SlotInvokerPtr executor) {
    assert(executor && s);
    return connect(combineSlotWithExecutor<Args_...>(move(s), move(executor)));
  }

  ConnectionType connect(ConnectionAwareSlotType cas) {
    auto ps = makeSlotPtr({});
    auto conPtr = ConnectionPtrType{new ConnectionType{sharedKeeper(), ps}};
    *ps = [cas{move(cas)},
           conPtr = move(conPtr)](ConstRef_<Args_>... args) mutable {
      cas(conPtr, args...);
      // Fix issue memory leadk due to cyclick reference
      // Slot holds a connection shared_ptr
      // this conPtr holds a slot shared_ptr after conPtr->disconnect() is
      // called
      if (conPtr->holdingConnector()) {
        conPtr->clear();
      }
    };
    return saveSlot(move(ps));
  }

  ConnectionType connect(ConnectionAwareSlotType cas, SlotInvokerPtr executor) {
    return connect(combineSlotWithExecutor<ConnectionPtrType, Args_...>(
        move(cas), move(executor)));
  }

  ConnectionType connect(TrackableObjPtrType obj, SlotType s) {
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

  ConnectionType connect(TrackableObjPtrType obj, SlotType s,
                         SlotInvokerPtr executor) {
    assert(executor && "executor must not be null");
    return connect(move(obj),
                   combineSlotWithExecutor<Args_...>(move(s), move(executor)));
  }

  template <
      class SlotLike,
      std::enable_if_t<is_constructible_v<SlotType, SlotLike>, bool> = true>
  ConnectionType connect(shared_ptr<SlotLike> ps) {
    assert(ps);
    return connect(ps, ref(*ps));
  }

  ConnectionType connect(SlotPtrType ps) {
    assert(ps && *ps);
    return saveSlot(ps);
  }

  ConnectionType connect(SlotPtrType ps, SlotInvokerPtr executor) {
    return connect(combineSlotWithExecutor<Args_...>(move(ps), move(executor)));
  }

  template <class _Keeper, class... _Args,
            enable_if_t<is_constructible_v<SlotType,
                                           function<void(ConstRef_<_Args>...)>>,
                        bool> = true>
  ConnectionType connect(BasicSignal_<_Keeper, _Args...>& otherSignal) {
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

  template <class _R, class _Class>
  static constexpr inline bool is_class_method_notifiable_v =
      std::is_invocable_v<_R(_Class::*), _Class*, Args_...>;
  template <class _R, class _Class>
  ConnectionType connect(_Class* ptr, _R(_Class::*method)) {
    static_assert(is_class_method_notifiable_v<_R, _Class>,
                  "Cannot bind method due to not invocable with signal's args");
    return this->connect(
        [ptr, method](ConstRef_<Args_>... args) { (ptr->*method)(args...); });
  }

  template <class _R, class _Class>
  ConnectionType connect(_Class* ptr, _R(_Class::*method),
                         SlotInvokerPtr executor) {
    static_assert(is_class_method_notifiable_v<_R, _Class>,
                  "Cannot bind method due to not invocable with signal's args");
    assert(executor);
    return this->connect(
        [ptr, method](ConstRef_<Args_>... args) { (ptr->*method)(args...); },
        move(executor));
  }

  template <class _R, class _Class>
  ConnectionType connect(const shared_ptr<_Class>& ptr, _R(_Class::*method)) {
    static_assert(is_class_method_notifiable_v<_R, _Class>,
                  "Cannot bind method due to not invocable with signal's args");
    return this->connect([wptr(weak_ptr(ptr)), method](
                             ConnectionPtrType con, ConstRef_<Args_>... args) {
      if (auto ptr = wptr.lock()) {
        (ptr.get()->*method)(args...);
      } else {
        con->disconnect();
      }
    });
  }

  template <class _R, class _Class>
  ConnectionType connect(const shared_ptr<_Class>& ptr, _R(_Class::*method),
                         SlotInvokerPtr executor) {
    static_assert(is_class_method_notifiable_v<_R, _Class>,
                  "Cannot bind method due to not invocable with signal's args");
    assert(executor);
    return this->connect(
        [wptr(weak_ptr(ptr)), method](ConnectionPtrType con,
                                      ConstRef_<Args_>... args) {
          if (auto ptr = wptr.lock()) {
            (ptr.get()->*method)(args...);
          } else {
            con->disconnect();
          }
        },
        move(executor));
  }

  bool disconnect(const SlotPtrType& ps) { return keeper()->remove(ps); }

  void disconnect() { keeper()->clear(); }

  bool connected() const { return keeper()->valid(); }

  size_t connectionCount() const { return keeper()->size(); }

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
      auto sparams = make_shared<tuple<PurgeType_<Ts>...>>(args...);
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

template <template <class, class...> class Signal_, class SlotKeeper_,
          typename... Args_>
class FutureInvocation {
  using Params = tuple<PurgeType_<Args_>...>;
  using SignalType = Signal_<SlotKeeper_, Args_...>;
  using ConnectionType = typename SignalType::ConnectionType;

 public:
  using SlotType = Slot_<Args_...>;
  FutureInvocation() = default;
  FutureInvocation(ConnectionType connection, SlotType s, future<Params> f)
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

template <template <class, class...> class Signal_, class SlotKeeper_,
          class... Args_>
[[nodiscard]] FutureInvocation<Signal_, SlotKeeper_, Args_...> waitableConnect(
    Signal_<SlotKeeper_, Args_...>& sig, Slot_<Args_...> sl) {
  using SignalType = Signal_<SlotKeeper_, Args_...>;
  using FutureType = FutureInvocation<Signal_, SlotKeeper_, Args_...>;
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
    FutureType ret(move(connection), move(sl), move(f));
    return ret;
  }

  return FutureType{};
}

class ScopedConnectionGroup : public vector<ScopedConnection> {
  template <class SlotsKeeper_, class... Args_>
  struct ConnectorHelper {
    using TheSignal_ = BasicSignal<SlotsKeeper_, Args_...>;
    ConnectorHelper(ScopedConnectionGroup& grp, TheSignal_& sig)
        : grp_(grp), sig_(sig) {}
    ConnectorHelper& with(typename TheSignal_::SlotType sl) {
      grp_ << sig_.connect(move(sl));
      return *this;
    }

   private:
    ScopedConnectionGroup& grp_;
    TheSignal_& sig_;
  };

 public:
  using ScopedConnectionType = ScopedConnection;
  ScopedConnectionGroup& operator<<(ScopedConnectionType&& conn) {
    push_back(move(conn));
    return *this;
  }

  template <class SlotsKeeper_, class... Args_>
  ConnectorHelper<SlotsKeeper_, Args_...> bind(
      BasicSignal<SlotsKeeper_, Args_...>& sig) {
    return ConnectorHelper(*this, sig);
  }
};

}  // namespace details

using details::Connection;
using details::ConnectionPtr;
using details::FutureInvocation;
using details::ScopedConnection;
using details::ScopedConnectionGroup;
using details::SCSignal;
using details::SCSignalST;
using details::Signal;
using details::SignalST;
using details::SlotInvokerPtr;
using details::waitableConnect;

namespace single_thread {

template <class... Args>
using Signal = signal_slots::details::SignalST<Args...>;
template <class... Args>
using SCSignal = signal_slots::details::SCSignalST<Args...>;

}  // namespace single_thread
}  // namespace signal_slots
}  // namespace maf
