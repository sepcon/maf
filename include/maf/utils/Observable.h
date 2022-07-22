#pragma once

#include "SignalSlots.h"

namespace maf {
namespace signal_slots {
namespace details {

template <template <class...> class KeeperBase_, class... SubStates_>
class StateBasedSlotKeeper : public KeeperBase_<SubStates_...> {
  using Base_ = KeeperBase_<SubStates_...>;

 public:
  using State = tuple<PurgeType_<SubStates_>...>;
  template <size_t idx>
  using SubState = tuple_element_t<idx, State>;

  void init(State state) { state_ = move(state); }

  void notifyTheLastSlot() { apply(*Base_::lastSlot(), state_); }
  void notifyOne(typename Base_::SlotPtrType const& sp) const {
    if (sp) {
      apply(*sp, state_);
    }
  }
  const State& get() const { return state_; }
  State& get() { return state_; }

  template <size_t idx>
  const SubState<idx>& get() const {
    return std::get<idx>(state_);
  }

  template <size_t idx>
  SubState<idx>& get() {
    return std::get<idx>(state_);
  }

  template <size_t... idx>
  void set(SubState<idx>... vals) {
    auto current = tie(std::get<idx>(state_)...);
    auto newVals = tie(vals...);
    if (current != newVals) {
      current = move(newVals);
      notifyState();
    }
  }

  void set(ConstRef_<SubStates_>... state) {
    auto newState = tie(state...);
    if (newState != state_) {
      state_ = newState;
      Base_::notify(state...);
    }
  }

  template <size_t... idx>
  void silentSet(SubState<idx>... vals) {
    tie(std::get<idx>(state_)...) = move(tie(vals...));
  }

  void silentSet(ConstRef_<SubStates_>... vals) { state_ = tie(vals...); }

  void notifyState() const {
    apply([this](
              ConstRef_<SubStates_>... newState) { this->notify(newState...); },
          state_);
  }

 private:
  State state_;
};

template <class... SubStates>
using S_StateBasedKeeper_ =
    StateBasedSlotKeeper<SingleSlotKeeper, SubStates...>;

template <class... SubStates>
using M_StateBasedKeeper_ = StateBasedSlotKeeper<MultiSlotKeeper, SubStates...>;

template <class T, class Derived_>
class ArithmeticStateBase_ {
 private:
  Derived_* derived() { return static_cast<Derived_*>(this); }
  const Derived_* derived() const { return static_cast<const Derived_*>(this); }

 public:
  void operator=(T v) { *derived()->mutable_() = v; }
  operator T() const { return (*derived()->immutable()); }
  T operator++() { return ++(*derived()->mutable_()); }
  T operator++(int) { return (*derived()->mutable_())++; }
  T operator+=(const T& other) { return (*derived()->mutable_()) += other; }
  T operator-=(const T& other) { return (*derived()->mutable_()) -= other; }
  T operator*=(const T& other) { return (*derived()->mutable_()) *= other; }
  T operator/=(const T& other) { return (*derived()->mutable_()) /= other; }
  T operator%=(const T& other) { return (*derived()->mutable_()) %= other; }
  T operator|=(const T& other) { return (*derived()->mutable_()) |= other; }
  T operator&=(const T& other) { return (*derived()->mutable_()) &= other; }
  T operator<<=(int bits) { return (*derived()->mutable_()) <<= bits; }
  T operator>>=(int bits) { return (*derived()->mutable_()) >>= bits; }
  T operator^=(const T& other) { return (*derived()->mutable_()) ^= other; }
};

template <class SlotsKeeper_, class... SubStates_>
class ObservableBasic_ : public BasicSignal<SlotsKeeper_, SubStates_...> {
  using Base_ = BasicSignal<SlotsKeeper_, SubStates_...>;
  using Base_::notify;
  using Base_::operator();
  static_assert(sizeof...(SubStates_) >= 1,
                "Observable must have at least one state");

 public:
  using SlotsKeeperType = SlotsKeeper_;
  using Base_::connect;
  using State = tuple<PurgeType_<SubStates_>...>;
  ObservableBasic_() = default;
  ObservableBasic_(ObservableBasic_&&) = default;
  ObservableBasic_& operator=(ObservableBasic_&&) = default;

  template <typename... _Args>
  Connection connect(_Args&&... args) {
    return connect_(std::forward<_Args>(args)...);
  }

  template <typename... _Args>
  Connection silentlyConnect(_Args&&... args) {
    return silentlyConnect_(std::forward<_Args>(args)...);
  }

  template <typename... _Args>
  Connection connect(_Args&&... args) const {
    return const_cast<ObservableBasic_*>(this)->connect_(
        std::forward<_Args>(args)...);
  }

  template <typename... _Args>
  Connection silentlyConnect(_Args&&... args) const {
    return const_cast<ObservableBasic_*>(this)->silentlyConnect_(
        std::forward<_Args>(args)...);
  }

 protected:
  template <typename... _Args>
  Connection connect_(_Args&&... args) {
    auto con = Base_::connect(std::forward<_Args>(args)...);
    this->keeper()->notifyOne(
        con.template getSlotPtr<typename Base_::SlotType>());
    return con;
  }

  template <typename... _Args>
  Connection silentlyConnect_(_Args&&... args) {
    return Base_::connect(std::forward<_Args>(args)...);
  }

  template <size_t idx>
  using SubState = tuple_element_t<idx, State>;

  template <size_t idx>
  struct SubStateRefBase_ {
    typename SlotsKeeper_::RefType atomickeeper_;
    SubState<idx> oldState_;
    SubState<idx>* operator->() {
      return addressof(atomickeeper_->template get<idx>());
    }

    SubState<idx>& operator*() { return atomickeeper_->template get<idx>(); }

    SubStateRefBase_(ObservableBasic_* o)
        : atomickeeper_(o->keeper().atomic()),
          oldState_(atomickeeper_->template get<idx>()) {}

    ~SubStateRefBase_() {
      if (oldState_ != operator*()) {
        atomickeeper_->notifyState();
      }
    }
  };

  template <size_t idx, typename = bool>
  struct SubStateRef : public SubStateRefBase_<idx> {};

  template <size_t idx>
  struct SubStateRef<idx, enable_if_t<is_arithmetic_v<SubState<idx>>, bool>>
      : public SubStateRefBase_<idx>,
        public ArithmeticStateBase_<SubState<idx>, SubStateRef<idx>> {
    auto& mutable_() { return *this; }
    using ArithmeticStateBase_<SubState<idx>, SubStateRef<idx>>::operator=;
  };

  template <size_t idx>
  struct CSubStateRef {
    typename SlotsKeeper_::CRefType atomickeeper_;

    const SubState<idx>* operator->() const {
      return addressof(atomickeeper_->template get<idx>());
    }

    const SubState<idx>& operator*() const {
      return atomickeeper_->template get<idx>();
    }

    CSubStateRef(const ObservableBasic_* o)
        : atomickeeper_(o->keeper().atomic()) {}
  };

  template <size_t idx>
  struct silentSubStateRef {
    typename SlotsKeeper_::RefType atomickeeper_;

    SubState<idx>* operator->() {
      return addressof(atomickeeper_->template get<idx>());
    }

    SubState<idx>& operator*() { return atomickeeper_->template get<idx>(); }

    silentSubStateRef(ObservableBasic_* o)
        : atomickeeper_(o->keeper().atomic()) {}
  };

 public:
  template <size_t idx>
  auto mutable_() {
    return SubStateRef<idx>{this};
  }
  template <size_t idx>
  auto silentMutable() {
    return silentSubStateRef<idx>{this};
  }

  template <size_t idx>
  auto immutable() const {
    return CSubStateRef<idx>{this};
  }
};

template <class SlotsKeeper_, class... SubStates_>
class BasicObservable_ : public ObservableBasic_<SlotsKeeper_, SubStates_...> {
  using Base_ = ObservableBasic_<SlotsKeeper_, SubStates_...>;

 public:
  using SlotsKeeperType = SlotsKeeper_;
  using Base_::Base_;
  using State = typename Base_::State;
  template <size_t idx>
  using SubState = typename Base_::template SubState<idx>;
  using Base_::operator=;
  using Base_::connect;
  using Base_::immutable;
  using Base_::mutable_;

  auto mutable_() { return StateRef<State>{this}; }
  auto immutable() const { return CStateRef{this}; }
  auto silentMutable() { return silentStateRef{this}; }

  BasicObservable_(SubStates_&&... subStates) {
    this->slotKeeper_->lockee().init(forward_as_tuple(subStates...));
  }

  BasicObservable_& operator=(const State& state) {
    apply([this](const SubStates_&... subStates) { set(subStates...); }, state);
    return *this;
  }

  void set(ConstRef_<SubStates_>... substates) {
    this->keeper()->set(substates...);
  }

  template <size_t... idx>
  void set(const SubState<idx>&... vals) {
    this->keeper()->template set<idx...>(vals...);
  }

  void silentSet(ConstRef_<SubStates_>... substates) {
    this->keeper()->silentSet(substates...);
  }

  template <size_t... idx>
  void silentSet(const SubState<idx>&... vals) {
    this->keeper()->template silentSet<idx...>(vals...);
  }

  State get() const { return this->keeper()->get(); }

  template <size_t idx>
  SubState<idx> get() const {
    return this->keeper()->template get<idx>();
  }

 private:
  template <class _State>
  struct StateRef {
   private:
    typename SlotsKeeper_::RefType atomickeeper_;
    _State oldState_;

   public:
    decltype(auto) operator->() { return addressof(atomickeeper_->get()); }

    template <size_t idx>
    decltype(auto) get() {
      return atomickeeper_->template get<idx>();
    }

    decltype(auto) operator*() { return atomickeeper_->get(); }

    StateRef(BasicObservable_* o)
        : atomickeeper_{o->keeper().atomic()},
          oldState_(atomickeeper_->get()) {}

    ~StateRef() {
      if (oldState_ != atomickeeper_->get()) {
        atomickeeper_->notifyState();
      }
    }
  };

  struct CStateRef {
   private:
    typename SlotsKeeper_::CRefType atomickeeper_;

   public:
    CStateRef(const BasicObservable_* o)
        : atomickeeper_{o->keeper().atomic()} {}

    decltype(auto) operator->() const {
      return addressof(this->atomickeeper_->template get());
    }

    decltype(auto) operator*() const {
      return this->atomickeeper_->template get();
    }

    template <size_t idx>
    decltype(auto) get() const {
      return atomickeeper_->template get<idx>();
    }
  };

  struct silentStateRef {
    typename SlotsKeeper_::RefType atomickeeper_;

    decltype(auto) operator->() {
      return addressof(this->atomickeeper_->template get());
    }

    decltype(auto) operator*() { return this->atomickeeper_->template get(); }

    template <size_t idx>
    decltype(auto) get() {
      return atomickeeper_->template get<idx>();
    }
  };
};

template <class SlotsKeeper_, class SingleState_>
class SingleStateObservableBasic_
    : public ObservableBasic_<SlotsKeeper_, SingleState_> {
  using Base_ = ObservableBasic_<SlotsKeeper_, SingleState_>;

 public:
  using Base_::Base_;
  using State = SingleState_;
  using SlotsKeeperType = SlotsKeeper_;

  SingleStateObservableBasic_() = default;
  SingleStateObservableBasic_(SingleStateObservableBasic_&&) = default;
  SingleStateObservableBasic_& operator=(SingleStateObservableBasic_&&) =
      default;

  SingleStateObservableBasic_(State&& state) {
    this->slotKeeper_->lockee().init(forward<SingleState_>(state));
  }

  SingleStateObservableBasic_& operator=(const State& state) {
    set(state);
    return *this;
  }

  void set(const State& state) { this->keeper()->template set<0>(state); }
  State get() const { return this->keeper()->template get<0>(); }

  operator State() const { return this->get(); }

  decltype(auto) operator->() const { return immutable(); }
  auto mutable_() { return typename Base_::template SubStateRef<0>{this}; }
  auto immutable() const {
    return typename Base_::template CSubStateRef<0>{this};
  }
  auto silentMutable() {
    return typename Base_::template silentSubStateRef<0>{this};
  }
};

template <class SlotsKeeper_, class SingleState_, typename = bool>
class BasicSingleStateObservable_
    : public SingleStateObservableBasic_<SlotsKeeper_, SingleState_> {
  using Base_ = SingleStateObservableBasic_<SlotsKeeper_, SingleState_>;

 public:
  using Base_::Base_;
  using Base_::operator=;
};

template <class SlotsKeeper_, class SingleState_>
class BasicSingleStateObservable_<
    SlotsKeeper_, SingleState_,
    enable_if_t<is_arithmetic_v<SingleState_>, bool>>
    : public SingleStateObservableBasic_<SlotsKeeper_, SingleState_>,
      public ArithmeticStateBase_<
          SingleState_,
          BasicSingleStateObservable_<SlotsKeeper_, SingleState_>> {
  using Base_ = SingleStateObservableBasic_<SlotsKeeper_, SingleState_>;

 public:
  using Base_::Base_;
  using Base_::operator=;
};

template <class SlotsKeeper_, class SingleState_>
class BasicObservable_<SlotsKeeper_, SingleState_>
    : public BasicSingleStateObservable_<SlotsKeeper_, SingleState_> {
  using Base_ = BasicSingleStateObservable_<SlotsKeeper_, SingleState_>;

 public:
  using Base_::Base_;
  using Base_::operator=;
};

template <class SlotsKeeper_, class State0_, class State1_, class... StateN_>
class BasicCompoundObservable_
    : public BasicObservable_<SlotsKeeper_, State0_, State1_, StateN_...> {
  using Base_ = BasicObservable_<SlotsKeeper_, State0_, State1_, StateN_...>;

  ScopedConnectionGroup connections_;
  // don't allow user to set dirrectly value to compound observable
  using Base_::set;

 public:
  BasicCompoundObservable_() = default;
  BasicCompoundObservable_(BasicCompoundObservable_&&) = default;
  BasicCompoundObservable_& operator=(BasicCompoundObservable_&&) = default;

  template <class Observable0_, class Observable1_, class... ObservableN_>
  BasicCompoundObservable_(Observable0_& o0, Observable1_& o1,
                           ObservableN_&... oN) {
    ProcessorConnector<Observable0_, Observable1_,
                       ObservableN_...>::template connect<0>(this, o0, o1,
                                                             oN...);
  }

  template <class Observable0_, class Observable1_, class... ObservableN_>
  BasicCompoundObservable_(const util::ExecutorIFPtr executor, Observable0_& o0,
                           Observable1_& o1, ObservableN_&... oN) {
    ProcessorConnector<Observable0_, Observable1_,
                       ObservableN_...>::template connect<0>(this, executor, o0,
                                                             o1, oN...);
  }

  template <class O, class... ON>
  struct ProcessorConnector {
    template <size_t IdxOfFirst, class O0_, class... ON_>
    static void connect(BasicCompoundObservable_* outter, O0_& o0, ON_&... oN) {
      outter->connections_ << o0.connect(
          [sharedKeeper =
               outter->sharedKeeper()](const typename O0_::State& o0State) {
            (*sharedKeeper)->template set<IdxOfFirst>(o0State);
          });

      ProcessorConnector<ON...>::template connect<IdxOfFirst + 1, ON_...>(
          outter, oN...);
    }

    template <size_t IdxOfFirst, class O0_, class... ON_>
    static void connect(BasicCompoundObservable_* outter,
                        const util::ExecutorIFPtr& executor, O0_& o0,
                        ON_&... oN) {
      outter->template set<IdxOfFirst>(o0.get());
      outter->connections_ << o0.connect(
          [sharedKeeper =
               outter->sharedKeeper()](const typename O0_::State& o0State) {
            (*sharedKeeper)->template set<IdxOfFirst>(o0State);
          },
          executor);

      ProcessorConnector<ON...>::template connect<IdxOfFirst + 1, ON_...>(
          outter, executor, oN...);
    }
  };

  template <class O>
  struct ProcessorConnector<O> {
    template <size_t Idx, class O_>
    static void connect(BasicCompoundObservable_* outter, O_& o) {
      outter->connections_ << o.connect([sharedKeeper = outter->sharedKeeper()](
                                            const typename O_::State& state) {
        (*sharedKeeper)->template set<Idx>(state);
      });
    }

    template <size_t Idx, class O_>
    static void connect(BasicCompoundObservable_* outter,
                        const util::ExecutorIFPtr& executor, O_& o) {
      outter->template set<Idx>(o.get());
      outter->connections_ << o.connect(
          [sharedKeeper =
               outter->sharedKeeper()](const typename O_::State& state) {
            (*sharedKeeper)->template set<Idx>(state);
          },
          executor);
    }
  };
};

template <typename>
struct IsObservable : std::false_type {};

template <typename... T>
struct IsObservable<BasicObservable_<T...>> : std::true_type {};

template <class... State_>
using CompoundObservable = BasicCompoundObservable_<
    AtomicObject<M_StateBasedKeeper_<State_...>, recursive_mutex>, State_...>;

template <class... State_>
using SCCompoundObservable = BasicCompoundObservable_<
    AtomicObject<S_StateBasedKeeper_<State_...>, recursive_mutex>, State_...>;

template <class... State_>
using CompoundObservableST =
    BasicCompoundObservable_<NonAtomicObject_<M_StateBasedKeeper_<State_...>>,
                             State_...>;

template <class... State_>
using SCCompoundObservableST =
    BasicCompoundObservable_<NonAtomicObject_<S_StateBasedKeeper_<State_...>>,
                             State_...>;

template <class... Observable_>
auto scjoinst(Observable_&... oN) {
  return SCCompoundObservableST<typename Observable_::State...>(oN...);
}

template <class... Observable_>
auto joinst(Observable_&... oN) {
  return CompoundObservableST<typename Observable_::State...>{oN...};
}

template <class... Observable_>
auto scjoin(Observable_&... oN) {
  return SCCompoundObservable<typename Observable_::State...>(oN...);
}

template <class... Observable_>
auto join(Observable_&... oN) {
  return CompoundObservable<typename Observable_::State...>(oN...);
}

template <class... Observable_>
auto scjoinst(const util::ExecutorIFPtr& executor, Observable_&... oN) {
  return SCCompoundObservableST<typename Observable_::State...>(executor,
                                                                oN...);
}

template <class... Observable_>
auto joinst(const util::ExecutorIFPtr& executor, Observable_&... oN) {
  return CompoundObservableST<typename Observable_::State...>(executor, oN...);
}

template <class... Observable_>
auto scjoin(const util::ExecutorIFPtr& executor, Observable_&... oN) {
  return SCCompoundObservable<typename Observable_::State...>(executor, oN...);
}

template <class... Observable_>
auto join(const util::ExecutorIFPtr& executor, Observable_&... oN) {
  return CompoundObservable<typename Observable_::State...>(executor, oN...);
}

template <class Mutex_, template <class...> class StateKeeper_,
          class... SubStates_>
class BasicObservableThreadSafe
    : public BasicObservable_<AtomicObject<StateKeeper_<SubStates_...>, Mutex_>,
                              SubStates_...> {
  using _Base =
      BasicObservable_<AtomicObject<StateKeeper_<SubStates_...>, Mutex_>,
                       SubStates_...>;

 public:
  using _Base::_Base;
  using _Base::operator=;
  BasicObservableThreadSafe(Mutex_&& mutex) {
    this->slotKeeper_->getMutex() = move(mutex);
  }

  BasicObservableThreadSafe(Mutex_&& mutex, SubStates_&&... subStates)
      : _Base(std::forward<SubStates_>(subStates)...) {
    this->slotKeeper_->getMutex() = move(mutex);
  }
};

template <class Mutex_, class... SubStates_>
using BasicObservableTS =
    BasicObservableThreadSafe<Mutex_, M_StateBasedKeeper_, SubStates_...>;

template <class Mutex_, class... SubStates_>
using SCBasicObservableTS =
    BasicObservableThreadSafe<Mutex_, S_StateBasedKeeper_, SubStates_...>;

template <class... SubStates_>
using SCObservableST =
    BasicObservable_<NonAtomicObject_<S_StateBasedKeeper_<SubStates_...>>,
                     SubStates_...>;

template <class... SubStates_>
using ObservableST =
    BasicObservable_<NonAtomicObject_<M_StateBasedKeeper_<SubStates_...>>,
                     SubStates_...>;

template <class... SubStates_>
using SCObservable = SCBasicObservableTS<recursive_mutex, SubStates_...>;

template <class... SubStates_>
using Observable = BasicObservableTS<recursive_mutex, SubStates_...>;

template <class... Args>
using SharedMutexObservable =
    BasicObservableTS<threading::MutexReference<std::recursive_mutex>, Args...>;
template <class... Args>
using SharedMutexSCObservable =
    BasicObservableTS<threading::MutexReference<std::recursive_mutex>, Args...>;

template <class... _Args, class _Callback>
auto waitableConnect(BasicObservable_<_Args...>& o, _Callback callback) {
  return waitableConnect(static_cast<BasicSignal<_Args...>&>(o),
                         move(callback));
}

}  // namespace details

using details::BasicObservableTS;
using details::CompoundObservable;
using details::CompoundObservableST;
using details::Observable;
using details::ObservableST;
using details::SCBasicObservableTS;
using details::SCCompoundObservable;
using details::SCCompoundObservableST;
using details::SCObservable;
using details::SCObservableST;
using details::SharedMutexObservable;
using details::SharedMutexSCObservable;

namespace mt {
template <class... SubStates>
using Observable = details::Observable<SubStates...>;

template <class... SubStates>
using SCObservable = details::SCObservable<SubStates...>;

template <class... SubStates>
using CompoundObservable = details::CompoundObservable<SubStates...>;

template <class... SubStates>
using SCCompoundObservable = details::SCCompoundObservable<SubStates...>;

template <class... Observable_>
auto join(Observable_&... oN) {
  return details::join(oN...);
}

template <class... Observable_>
auto scjoin(Observable_&... oN) {
  return details::scjoin(oN...);
}

template <class... Observable_>
auto join(const util::ExecutorIFPtr& executor, Observable_&... oN) {
  return details::join(executor, oN...);
}

template <class... Observable_>
auto scjoin(const util::ExecutorIFPtr& executor, Observable_&... oN) {
  return details::scjoin(executor, oN...);
}

}  // namespace mt

namespace st {
template <class... SubStates>
using Observable = details::ObservableST<SubStates...>;

template <class... SubStates>
using SCObservable = details::SCObservableST<SubStates...>;

template <class... SubStates>
using CompoundObservable = details::CompoundObservableST<SubStates...>;

template <class... SubStates>
using SCCompoundObservable = details::SCCompoundObservableST<SubStates...>;

template <class... Observable_>
auto join(Observable_&... oN) {
  return details::joinst(oN...);
}

template <class... Observable_>
auto scjoin(Observable_&... oN) {
  return details::scjoinst(oN...);
}

template <class... Observable_>
auto join(const util::ExecutorIFPtr& executor, Observable_&... oN) {
  return details::joinst(executor, oN...);
}

template <class... Observable_>
auto scjoin(const util::ExecutorIFPtr& executor, Observable_&... oN) {
  return details::scjoinst(executor, oN...);
}

}  // namespace st

}  // namespace signal_slots
}  // namespace maf
