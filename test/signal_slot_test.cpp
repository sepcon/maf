
#include <maf/Messaging.h>
#include <maf/utils/DirectExecutor.h>
#include <maf/utils/Observable.h>
#include <maf/utils/SignalSlots.h>

#include <map>
#include <thread>

#define CATCH_CONFIG_MAIN
#include "catch/catch_amalgamated.hpp"

using namespace maf;
using namespace maf::messaging;
using namespace maf::signal_slots;
using namespace maf::threading;
using namespace maf::util;
using namespace std;

TEST_CASE("shared_mutex_signal") {
  std::recursive_mutex mutex;
  SharedMutexSignal<int> sigint{mutex};
  SharedMutexSignal<std::string> sigstr{mutex};
  //  Signal<int> sigint;           |--> will crash if enable
  //  Signal<std::string> sigstr;   |-->

  int int_ = 0;
  string str_;
  sigint.connect([&](int val) {
    int_ = val;
    sigstr.connect([&](std::string const& val) {
      str_ = val;
      sigint.connect([](int) {});  // won't crash here
    });
    sigstr(std::to_string(val));
  });

  sigint(100);
  REQUIRE(int_ == 100);
  REQUIRE(str_ == "100");

  SharedMutexObservable<int> stateInt{mutex};
  SharedMutexObservable<std::string> stateStr{mutex};
  // will crash with below 2 observables
  //  SharedMutexObservable<int> stateInt;
  //  SharedMutexObservable<std::string> stateStr;

  stateInt.connect([&](int val) {
    int_ = val;
    stateStr.connect([&](std::string const& val) {
      str_ = val;
      stateInt.connect([](int) {});  // won't crash here
    });
    stateStr = std::to_string(val);
  });
  stateInt = 500;
  REQUIRE(int_ == 500);
  REQUIRE(str_ == "500");
}

TEST_CASE("single_connection_single_thread_Test") {
  SCSignalST<> sig;
  bool fired = false;
  bool fired2 = false;
  auto slot1 = [&fired] { fired = true; };
  auto slot2 = [&fired2] { fired2 = true; };

  sig.connect(slot1);

  auto con2 = sig.connect(slot2);
  REQUIRE(!con2.connected());  // only one slot can connect

  sig();
  REQUIRE(fired);
  fired = false;
  sig.disconnect();
  sig();

  auto con = sig.connect(slot1);
  sig();
  REQUIRE(fired);
  con.disconnect();
  fired = false;
  sig();
  REQUIRE(!fired);
}

TEST_CASE("multiconnection_single_thread_test") {
  SignalST<> sig;
  bool fired1 = false;
  bool fired2 = false;
  auto reset = [&fired1, &fired2] { fired1 = fired2 = false; };

  auto slot1 = [&fired1] { fired1 = true; };
  auto slot2 = [&fired2] { fired2 = true; };

  SECTION("multi_connection_signal_st") {
    auto con1 = sig.connect(slot1);
    auto con2 = sig.connect(slot2);
    REQUIRE(con1.connected());
    REQUIRE(con2.connected());

    sig();
    REQUIRE(fired1);
    REQUIRE(fired2);

    reset();

    con1.disconnect();
    REQUIRE(!con1.connected());

    sig();

    REQUIRE(!fired1);
    REQUIRE(fired2);

    fired2 = false;
    con2.disconnect();

    sig();
    REQUIRE(!fired1);
    REQUIRE(!fired2);

    reset();
    sig.connect(slot1);
    sig.connect(slot2);
    sig();
    REQUIRE(fired1);
    REQUIRE(fired2);

    sig.disconnect();  // disconnect all connections
    reset();
    sig();
    REQUIRE(!fired1);
    REQUIRE(!fired2);
  }

  SECTION("connection_disconnect_reconnect") {
    Signal<int> sigInt;
    auto notifiedInt = 0;
    auto con = sigInt.connect([&notifiedInt](int val) { notifiedInt = val; });

    sigInt.notify(10);
    REQUIRE(notifiedInt == 10);
    con.disconnect();
    sigInt.notify(100);
    REQUIRE(notifiedInt == 10);
    con.reconnect();
    sigInt.notify(-1);
    REQUIRE(notifiedInt == -1);
  }
}

TEST_CASE("multi_connection_multi_thread_test") {
  using Ints = vector<size_t>;
  Signal<size_t, Ints> dataReadySignal;
  Signal<> receiverReadySignal;
  Signal<> receiverStopSignal;
  Signal<> outOfDataSignal;

  using ReceivedData = multimap<size_t, Ints>;
  auto rd1 = make_shared<ReceivedData>();
  auto rd2 = make_shared<ReceivedData>();

  auto onDataReady1 = [rd1](size_t size, Ints vec) {
    rd1->emplace(size, move(vec));
  };
  auto onDataReady2 = [rd2](size_t size, Ints vec) {
    rd2->emplace(size, move(vec));
  };

  auto emitDataReadySignal = [&] {
    for (size_t i = 0; i < 10; ++i) {
      dataReadySignal.notify(i, {i});
    }

    outOfDataSignal.notify();
  };

  auto comp1 = Processor::create("thread1");
  AsyncProcessor comp2 = Processor::create("thread2");

  comp2.launch([&] {
    dataReadySignal.connect(onDataReady1, this_processor::getExecutor());
    dataReadySignal.connect(onDataReady2, this_processor::getExecutor());
    receiverReadySignal();
  });

  receiverReadySignal.connect(emitDataReadySignal, comp1->getExecutor());

  outOfDataSignal.connect(
      [&] {
        this_processor::stop();
        receiverStopSignal.notify();
      },
      comp2->getExecutor());

  receiverStopSignal.connect([] { this_processor::stop(); },
                             comp1->getExecutor());

  comp1->run();

  REQUIRE(rd1->size() > 0);
  REQUIRE(*rd1 == *rd2);
}

TEST_CASE("scoped_connection_test") {
  Signal<int> sig;
  int normalInt = 0;
  int scopedInt = 0;
  int scopedInt1 = 0;

  {
    auto con = sig.connect([&normalInt](int val) { normalInt = val; });

    ScopedConnection scon = sig.connect([&scopedInt](int i) { scopedInt = i; });
    {
      ScopedConnection scon1 =
          sig.connect([&scopedInt1](int x) { scopedInt1 = x; });
      sig.notify(10);

      REQUIRE(scopedInt == 10);
      REQUIRE(scopedInt1 == 10);
      REQUIRE(normalInt == 10);
    }

    // out of scope, then scopedInt must still be 10 after emit signal again
    sig.notify(100);
    //      REQUIRE(scopedInt == 100 ); REQUIRE( scopedInt1 == 10 ); REQUIRE(
    //      normalInt == 100);
    REQUIRE(scopedInt == 100);
    REQUIRE(normalInt == 100);
    REQUIRE(scopedInt1 == 10);
  }

  sig.notify(1000);

  REQUIRE(scopedInt == 100);
  REQUIRE(normalInt == 1000);
  REQUIRE(scopedInt1 == 10);

  sig.disconnect();
  sig.notify(10000);
  REQUIRE(normalInt == 1000);
}

TEST_CASE("connection_aware_slot_test") {
  Signal<int> sigInt;
  int testVal = 0;
  int testVal1 = 0;
  sigInt.connect(
      [&testVal](const Signal<int>::ConnectionPtrType& con, int val) {
        con->disconnect();
        testVal = val;
      },
      directExecutor());

  auto con = sigInt.connect([&testVal1](auto con, int val) {
    con->disconnect();
    testVal1 = val;
  });

  sigInt(1000);
  REQUIRE(testVal == 1000);
  REQUIRE(testVal1 == 1000);
  REQUIRE(!con.connected());

  sigInt(-1);
  REQUIRE(testVal == 1000);
  REQUIRE(testVal1 == 1000);
}

TEST_CASE("connection_disconnect_other_slots_when_being_notified_test") {
  Signal<> sig;
  int notifyCount = 0;
  shared_ptr<Connection> con1, con2;
  con1 = make_shared<Connection>(sig.connect([&] {
    if (con2) {
      con2->disconnect();
      con2.reset();
    }
    ++notifyCount;
  }));
  con2 = make_shared<Connection>(sig.connect([&] { notifyCount++; }));
  sig();
  REQUIRE(notifyCount == 1);
}

TEST_CASE("scoped_connection_group_test") {
  Signal<> sig;
  Signal<int> sigint;
  int notifyCount = 0;
  auto lambda1 = [&] { ++notifyCount; };
  auto lambda2 = [&] { ++notifyCount; };
  {
    ScopedConnectionGroup connections;
    connections.bind(sig).with(lambda1).with(lambda2).with(
        [&] { ++notifyCount; });
    connections.bind(sigint).with([&](int) { ++notifyCount; });
    connections << sigint.connect([&](int) { ++notifyCount; });
  }

  sig();
  sigint(100);
  REQUIRE(notifyCount == 0);
}

TEST_CASE("connect_with_class_method_test") {
  static int triggerCount = 0;
  struct Hello {
    void onTriggered(double) { ++triggerCount; }
  };

  Signal<int> intsig;
  auto h = make_shared<Hello>();
  intsig.connect(h, &Hello::onTriggered);
  intsig.notify(1);
  intsig.notify(1);
  intsig.notify(1);
  REQUIRE(triggerCount == 3);

  h.reset();  // delete Hello and must not received notification
  intsig.connect([](double) { ++triggerCount; });
  intsig(1);
  REQUIRE(triggerCount == 4);
}

TEST_CASE("trackable_slot_test") {
  SignalST<int> sig;
  struct Invokable {
    int val = 0;
    void setValue(int val) { this->val = val; }
  };

  shared_ptr<Invokable> pi = make_shared<Invokable>();
  int ii = 0;

  sig.connect(pi, [&ii, p = pi.get()](int i) {
    p->setValue(i);
    ii = i;
  });

  sig(1000);
  REQUIRE(ii == 1000);

  pi.reset();
  // after pi was destroyed, it should nolonger receied signal
  sig(3000);
  REQUIRE(ii == 1000);
}

TEST_CASE("observable_single_state_test") {
  Observable<int> state;
  Observable<vector<int>> vec;

  int observedState = 0;
  auto pStr = make_shared<string>("hello world");
  state.connect(
      pStr, [p = pStr.get()](int state) { *p = to_string(state); },
      maf::util::directExecutor());

  const auto& cstate = state;
  cstate.connect([&](int val) { observedState = val; });

  SECTION("observable_state_pod") {
    state = 1;
    REQUIRE(observedState == 1);
    REQUIRE(state.get() == 1);

    state = 2;
    REQUIRE(observedState == 2);
    REQUIRE(state.get() == 2);

    REQUIRE(*pStr == "2");

    pStr.reset();
    // must not crash here
    state = 3;
    REQUIRE(observedState == 3);
    REQUIRE(state.get() == 3);
  }

  SECTION("observable_state_class") {
    std::vector<int> observedVector;
    int notifyCount = 0;
    vec.connect([&](auto vec) {
      ++notifyCount;
      observedVector = move(vec);
    });

    vec.mutable_()->assign(3, 10);
    REQUIRE(observedVector == vector<int>{10, 10, 10});
    REQUIRE(notifyCount == 2);
    static_cast<void>(vec->size());  // call immutable methods won't notify
    REQUIRE((observedVector == vector<int>{10, 10, 10}));
    REQUIRE(notifyCount == 2);
    {
      auto mvec = vec.mutable_();
      mvec->clear();
      mvec->push_back(100);
      mvec->insert(mvec->begin(), 1000);
    }

    REQUIRE((observedVector == vector<int>{1000, 100}));

    {
      notifyCount = 0;
      auto mvec = vec.mutable_();
      mvec->clear();
      mvec->push_back(100);
      mvec->insert(mvec->begin(), 1000);
    }
    REQUIRE(notifyCount == 0);
  }
}

template <class Observable_>
void arithmetic_observable_test_impl() {
  Observable_ state;
  auto notifyCount = 0;
  auto observedState = 0;
  state.connect([&](auto val) {
    ++notifyCount;
    observedState = val;
  });

  state = 3;
  ++state;
  REQUIRE(observedState == 4);
  REQUIRE(state.get() == 4);
  state++;
  REQUIRE(observedState == 5);
  REQUIRE(state.get() == 5);
  state += 1;
  REQUIRE(observedState == 6);
  REQUIRE(state.get() == 6);
  state -= 1;
  REQUIRE(observedState == 5);
  REQUIRE(state.get() == 5);
  state *= 2;
  REQUIRE(observedState == 10);
  REQUIRE(state.get() == 10);
  state /= 2;
  REQUIRE(observedState == 5);
  REQUIRE(state.get() == 5);
  state <<= 2;
  REQUIRE(observedState == 5 << 2);
  REQUIRE(state.get() == 5 << 2);
  state >>= 2;
  REQUIRE(observedState == 5);
  REQUIRE(state.get() == 5);

  state = 20;
  // won't notify
  auto newVal = state + 10;
  REQUIRE(state.get() == 20);
  REQUIRE(observedState == 20);
  newVal = state - 10;
  REQUIRE(state.get() == 20);
  REQUIRE(observedState == 20);
  newVal = state * 10;
  REQUIRE(state.get() == 20);
  REQUIRE(observedState == 20);
  newVal = state / 10;
  REQUIRE(state.get() == 20);
  REQUIRE(observedState == 20);
  newVal = state << 2;
  REQUIRE(state.get() == 20);
  REQUIRE(observedState == 20);
  newVal = state >> 2;
  REQUIRE(state.get() == 20);
  REQUIRE(observedState == 20);
  REQUIRE(newVal == (20 >> 2));
}

TEST_CASE("arithmetic_observable_test") {
  arithmetic_observable_test_impl<Observable<int>>();
  arithmetic_observable_test_impl<ObservableST<int>>();
  arithmetic_observable_test_impl<SCObservableST<int>>();
  arithmetic_observable_test_impl<SCObservable<int>>();
}

TEST_CASE("observable_multi_state_test") {
  Observable<int, string> state;
  int iVal = -1;
  string sVal = "";
  auto resetExpected = [&] {
    iVal = -1;
    sVal = "";
  };

  state.connect([&](int i, const string& s) {
    iVal = i;
    sVal = s;
  });

  SECTION("observable_state") {
    state.set(1, "1");
    REQUIRE(iVal == 1);
    REQUIRE(state.get<0>() == 1);
    REQUIRE(sVal == "1");
    REQUIRE(state.get<1>() == "1");

    resetExpected();

    state.set(1, "1");

    REQUIRE(iVal == -1);
    REQUIRE(state.get<0>() == 1);
    REQUIRE(sVal == "");

    resetExpected();
    state.set(2, "1");

    REQUIRE(iVal == 2);
    REQUIRE(state.get<0>() == 2);
    REQUIRE(sVal == "1");

    resetExpected();
    state.set(2, "2");

    REQUIRE(iVal == 2);
    REQUIRE(state.get<0>() == 2);
    REQUIRE(sVal == "2");

    state.mutable_<1>()->push_back('3');
    state.mutable_<0>()++;
    REQUIRE(iVal == 3);
    REQUIRE(sVal == "23");

    state.mutable_<0>() += 2;
    REQUIRE(iVal == 5);

    state.mutable_<0>() *= 2;
    REQUIRE(iVal == 10);
  }

  SECTION("observable_multi_state_concurrent_set") {
    Observable<string, bool, int> sb;
    int notifyCount = 0;
    sb.connect([&](const string&, bool, int) { ++notifyCount; });

    {
      auto msb = sb.mutable_();
      msb.get<0>().push_back('1');
      msb.get<1>() = false;
      msb.get<1>() = true;
      msb.get<1>() = false;
      msb.get<1>() = true;
      msb.get<2>() += 1;
      msb.get<2>() *= 100;
    }

    REQUIRE(notifyCount == 2);

    sb.set<0>("hello");

    {
      notifyCount = 0;
      auto first = sb.mutable_<0>();
      *first += "hello";
      first->clear();
      *first += "hello";
    }
    REQUIRE(notifyCount == 0);

    {
      notifyCount = 0;
      auto first = sb.mutable_<0>();
      first->clear();
      first->assign("bello world");
    }
    REQUIRE(notifyCount == 1);
    REQUIRE(sb.get<0>() == "bello world");
  }

  SECTION("observable_multi_state_sub_states_set") {
    Observable<int, bool, string> o{-1, false, ""};
    int ei = 0;
    bool eb = false;
    string es;
    int notifiedCount = 0;
    o = tuple{-1, false, ""};
    o.connect([&](int i, bool b, const string& s) {
      ei = i;
      eb = b;
      es = s;
      ++notifiedCount;
    });

    o.set<1, 2>(true, "true");
    REQUIRE(ei == -1);
    REQUIRE(eb);
    REQUIRE(es == "true");

    o.set<0, 2>(100, "false");
    REQUIRE(ei == 100);
    REQUIRE(eb);
    REQUIRE(es == "false");

    o.set<1, 0>(false, 10);
    REQUIRE(ei == 10);
    REQUIRE(!eb);
    REQUIRE(es == "false");
  }
}

TEST_CASE("compound_observable_test") {
  enum MouseStatus {
    LeftPressed,
    RightPressed,
    MiddlePressed,
    LeftReleased,
    RightReleased,
    MiddleReleased,
    AllReleased
  };

  Observable<string> pressingKey{""};
  Observable<MouseStatus> mouseStatus{AllReleased};
  auto km = mt::join(pressingKey, mouseStatus);
  bool jump = false;
  int notified = 0;
  km.connect([&](const string& key, MouseStatus mouseStatus) {
    ++notified;
    jump = key == "ctrl" && mouseStatus == LeftPressed;
  });

  pressingKey = "ctrl";
  mouseStatus = MiddlePressed;
  REQUIRE(jump == false);
  REQUIRE(notified == 3);

  mouseStatus = LeftReleased;

  REQUIRE(jump == false);
  REQUIRE(notified == 4);
  mouseStatus = LeftPressed;
  REQUIRE(jump == true);
  REQUIRE(notified == 5);
  pressingKey = "q";
  REQUIRE(jump == false);
  REQUIRE(notified == 6);
}

TEST_CASE("connect_to_other_signal_test") {
  Signal<const char*> rootSignal;
  string expectedString;
  Connection con;
  {
    SignalST<string> triggeredSignal;
    con = rootSignal.connect(triggeredSignal);
    triggeredSignal.connect([&](const string& val) { expectedString = val; });

    REQUIRE(con.connected());
    REQUIRE(expectedString == "");
    rootSignal("100");
    REQUIRE(expectedString == "100");
  }

  rootSignal("300");
  REQUIRE(!con.connected());
  REQUIRE(expectedString == "100");
}
TEST_CASE("performance_test") {
  Signal<string> stringSignal;
  const string* s1 = nullptr;
  const string* s2 = nullptr;
  stringSignal.connect([&s1](const string& s) { s1 = &s; });
  stringSignal.connect([&s2](const string& s) { s2 = &s; });

  string originValue = "helloworld";
  stringSignal(originValue);
  REQUIRE(&originValue == s1);
  REQUIRE(s1 == s2);
}

TEST_CASE("observable_silent_set") {
  Observable<string, bool> ovb{"s", true};
  int notifyCount = 0;
  ovb.connect([&](const string&, bool) { ++notifyCount; });
  ovb.silentSet<1>(false);
  REQUIRE(notifyCount == 1);
  ovb.silentSet<1>(true);
  REQUIRE(notifyCount == 1);
  ovb.silentSet<0>("s1");
  REQUIRE(notifyCount == 1);
  ovb.silentSet("s2", false);
  REQUIRE(notifyCount == 1);

  Observable<string> sovb;
  sovb.silentlyConnect([&](const string&) { notifyCount++; });
  REQUIRE(notifyCount == 1);
  sovb.set("hello");
  REQUIRE(notifyCount == 2);
  sovb.set("hello1");
  REQUIRE(notifyCount == 3);

  Observable<string, bool> sovb2{"s", true};
  sovb2.silentlyConnect([&](const string&, auto) { notifyCount++; });
  REQUIRE(notifyCount == 3);
  sovb2.set<0>("s1");
  REQUIRE(notifyCount == 4);
  sovb2.set<1>(false);
  REQUIRE(notifyCount == 5);
}

TEST_CASE("multiple_thread_connect_to_observable") {
  maf::signal_slots::details::BasicObservable_<
      maf::threading::AtomicObject<
          maf::signal_slots::details::M_StateBasedKeeper_<string>, mutex>,
      string>
      os;
  atomic_int notifyCount = 0;
  vector<thread> threads;
  for (int i = 0; i < 10; ++i) {
    threads.emplace_back([&] {
      for (int c = 0; c < 100; ++c) {
        os.connect([&](const std::string& s) { ++notifyCount; });
      }
    });
  }
  for (auto& th : threads) {
    th.join();
  }
  REQUIRE(notifyCount == 1000);
  REQUIRE(os.connectionCount() == 1000);
}

TEST_CASE("observable_with_executor") {
  auto runner = Processor::create();
  Observable<string> str;
  int notifyCount = 0;
  runner->run([&] {
    str.connect(
        [&notifyCount](auto const&) {
          ++notifyCount;
          this_processor::stop();
        },
        this_processor::getExecutor());
    REQUIRE(notifyCount == 1);
  });

  Observable<string, string> str1;
  auto mstr0 = str1.mutable_<0>();
  mstr0->resize(1000);
}

TEST_CASE("waitable_connect") {
  Signal<bool> sig;
  auto ret = false;
  auto f = waitableConnect(sig, [&](bool) { ret = true; });
  sig(false);
  REQUIRE(ret == false);
  f.wait();
  REQUIRE(ret == true);
}
