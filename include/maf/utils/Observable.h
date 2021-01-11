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

  bool add(typename Base_::SlotPtrType ps) {
    auto& s = *ps;
    auto ret = Base_::add(move(ps));
    apply(s, state_);
    return ret;
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
  template <size_t idx>
  void set(SubState<idx> val) {
    if (val != std::get<idx>(state_)) {
      std::get<idx>(state_) = move(val);
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
  void operator=(T v) { *derived()->mut() = v; }
  operator T() const { return (*derived()->immut()); }
  T operator++() { return ++(*derived()->mut()); }
  T operator++(int) { return (*derived()->mut())++; }
  T operator+=(const T& other) { return (*derived()->mut()) += other; }
  T operator-=(const T& other) { return (*derived()->mut()) -= other; }
  T operator*=(const T& other) { return (*derived()->mut()) *= other; }
  T operator/=(const T& other) { return (*derived()->mut()) /= other; }
  T operator%=(const T& other) { return (*derived()->mut()) %= other; }
  T operator|=(const T& other) { return (*derived()->mut()) |= other; }
  T operator&=(const T& other) { return (*derived()->mut()) &= other; }
  T operator<<=(int bits) { return (*derived()->mut()) <<= bits; }
  T operator>>=(int bits) { return (*derived()->mut()) >>= bits; }
  T operator^=(const T& other) { return (*derived()->mut()) ^= other; }
};

template <class SlotsKeeper_, class... SubStates_>
class ObservableBasic_ : public BasicSignal_<SlotsKeeper_, SubStates_...> {
  using Base_ = BasicSignal_<SlotsKeeper_, SubStates_...>;
  using Base_::notify;
  using Base_::operator();

 public:
  using SlotsKeeperType = SlotsKeeper_;
  using Base_::connect;
  using State = tuple<PurgeType_<SubStates_>...>;
  ObservableBasic_() = default;
  ObservableBasic_(ObservableBasic_&&) = default;
  ObservableBasic_& operator=(ObservableBasic_&&) = default;

 protected:
  template <size_t idx>
  using SubState = tuple_element_t<idx, State>;

  template <size_t idx>
  struct SubStateRefBase_ {
    typename SlotsKeeper_::AtomicRefType atomickeeper_;
    bool modified = false;
    SubState<idx>* operator->() {
      modified = true;
      return addressof(atomickeeper_->template get<idx>());
    }

    SubState<idx>& operator*() {
      modified = true;
      return atomickeeper_->template get<idx>();
    }

    SubStateRefBase_(ObservableBasic_* o)
        : atomickeeper_(o->keeper().atomic()) {}
    ~SubStateRefBase_() {
      if (modified) {
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
    auto& mut() { return *this; }
    using ArithmeticStateBase_<SubState<idx>, SubStateRef<idx>>::operator=;
  };

  template <size_t idx>
  struct CSubStateRef {
    typename SlotsKeeper_::CAtomicRefType atomickeeper_;

    const SubState<idx>* operator->() const {
      return addressof(atomickeeper_->template get<idx>());
    }

    const SubState<idx>& operator*() const {
      return atomickeeper_->template get<idx>();
    }

    CSubStateRef(const ObservableBasic_* o)
        : atomickeeper_(o->keeper().atomic()) {}
  };

 public:
  template <size_t idx>
  auto mut() {
    return SubStateRef<idx>{this};
  }

  template <size_t idx>
  auto immut() const {
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
  using Base_::immut;
  using Base_::mut;

  auto mut() { return StateRef{this}; }
  auto immut() const { return CStateRef{this}; }

  BasicObservable_(SubStates_&&... subStates) {
    this->keeper()->init(forward_as_tuple(subStates...));
  }

  BasicObservable_& operator=(const State& state) {
    apply([this](const SubStates_&... subStates) { set(subStates...); }, state);
    return *this;
  }

  void set(ConstRef_<SubStates_>... substates) {
    this->keeper()->set(substates...);
  }

  template <size_t idx>
  void set(const SubState<idx>& val) {
    this->keeper()->template set<idx>(val);
  }

  State get() const { return this->keeper()->get(); }

  template <size_t idx>
  SubState<idx> get() const {
    return this->keeper()->template get<idx>();
  }

  typename Base_::Connection connect(function<void(const State&)> tpSl) {
    assert(tpSl);
    return connect([tpSl{move(tpSl)}](ConstRef_<SubStates_>... args) {
      tpSl(tie(args...));
    });
  }

  typename Base_::Connection connect(function<void(const State&)> tpSl,
                                     util::ExecutorIFPtr executor) {
    assert(tpSl);
    return connect([tpSl{move(tpSl)}](
                       ConstRef_<SubStates_>... args) { tpSl(tie(args...)); },
                   move(executor));
  }

 private:
  struct StateRef {
   private:
    typename SlotsKeeper_::AtomicRefType atomickeeper_;
    bool modified = false;

   public:
    decltype(auto) operator->() {
      modified = true;
      return addressof(atomickeeper_->get());
    }

    template <size_t idx>
    decltype(auto) get() {
      modified = true;
      return atomickeeper_->template get<idx>();
    }

    decltype(auto) operator*() {
      modified = true;
      return atomickeeper_->get();
    }

    StateRef(BasicObservable_* o) : atomickeeper_{o->keeper().atomic()} {}

    ~StateRef() {
      if (modified) {
        atomickeeper_->notifyState();
      }
    }
  };

  struct CStateRef {
    typename SlotsKeeper_::CAtomicRefType atomickeeper_;

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
    this->keeper()->init(forward<SingleState_>(state));
  }

  SingleStateObservableBasic_& operator=(const State& state) {
    set(state);
    return *this;
  }
  void set(const State& state) { this->keeper()->template set<0>(state); }
  State get() const { return this->keeper()->template get<0>(); }

  decltype(auto) operator->() const { return immut(); }
  auto mut() { return typename Base_::template SubStateRef<0>{this}; }
  auto immut() const { return typename Base_::template CSubStateRef<0>{this}; }
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
    ComponentConnector<Observable0_, Observable1_,
                       ObservableN_...>::template connect<0>(this, o0, o1,
                                                             oN...);
  }

  template <class Observable0_, class Observable1_, class... ObservableN_>
  BasicCompoundObservable_(const util::ExecutorIFPtr executor, Observable0_& o0,
                           Observable1_& o1, ObservableN_&... oN) {
    ComponentConnector<Observable0_, Observable1_,
                       ObservableN_...>::template connect<0>(this, executor, o0,
                                                             o1, oN...);
  }

  template <class O, class... ON>
  struct ComponentConnector {
    template <size_t IdxOfFirst, class O0_, class... ON_>
    static void connect(BasicCompoundObservable_* outter, O0_& o0, ON_&... oN) {
      outter->connections_ << o0.connect(
          [sharedKeeper =
               outter->sharedKeeper()](const typename O0_::State& o0State) {
            (*sharedKeeper)->template set<IdxOfFirst>(o0State);
          });

      ComponentConnector<ON...>::template connect<IdxOfFirst + 1, ON_...>(
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

      ComponentConnector<ON...>::template connect<IdxOfFirst + 1, ON_...>(
          outter, executor, oN...);
    }
  };

  template <class O>
  struct ComponentConnector<O> {
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

template <class... SubStates_>
using SCObservableST =
    BasicObservable_<NonAtomicObject_<S_StateBasedKeeper_<SubStates_...>>,
                     SubStates_...>;

template <class... SubStates_>
using ObservableST =
    BasicObservable_<NonAtomicObject_<M_StateBasedKeeper_<SubStates_...>>,
                     SubStates_...>;

template <class... SubStates_>
using SCObservable = BasicObservable_<
    AtomicObject<S_StateBasedKeeper_<SubStates_...>, recursive_mutex>,
    SubStates_...>;

template <class... SubStates_>
using Observable = BasicObservable_<
    AtomicObject<M_StateBasedKeeper_<SubStates_...>, recursive_mutex>,
    SubStates_...>;

}  // namespace details

using details::CompoundObservable;
using details::CompoundObservableST;
using details::Observable;
using details::ObservableST;
using details::SCCompoundObservable;
using details::SCCompoundObservableST;
using details::SCObservable;
using details::SCObservableST;

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
