
#include <maf/Messaging.h>
#include <maf/utils/DirectExecutor.h>
#include <maf/utils/Observable.h>
#include <maf/utils/SignalSlots.h>

#include <map>
#include <thread>

#include "test.h"

using namespace maf;
using namespace maf::messaging;
using namespace maf::signal_slots;
using namespace maf::util;
using namespace std;

void single_connection_single_thread_Test() {
  SCSignalST<> sig;
  bool fired = false;
  bool fired2 = false;
  auto slot1 = [&fired] { fired = true; };
  auto slot2 = [&fired2] { fired2 = true; };

  TEST_CASE_B(single_connection_signal_st) {
    sig.connect(slot1);

    auto con2 = sig.connect(slot2);
    EXPECT(!con2.connected());  // only one slot can connect

    sig();
    EXPECT(fired);
    fired = false;
    sig.disconnect();
    sig();

    auto con = sig.connect(slot1);
    sig();
    EXPECT(fired);
    con.disconnect();
    fired = false;
    sig();
    EXPECT(!fired);
  }
  TEST_CASE_E()
}

void multiconnection_single_thread_test() {
  SignalST<> sig;
  bool fired1 = false;
  bool fired2 = false;
  auto reset = [&fired1, &fired2] { fired1 = fired2 = false; };

  auto slot1 = [&fired1] { fired1 = true; };
  auto slot2 = [&fired2] { fired2 = true; };

  TEST_CASE_B(multi_connection_signal_st) {
    auto con1 = sig.connect(slot1);
    auto con2 = sig.connect(slot2);
    EXPECT(con1.connected());
    EXPECT(con2.connected());

    sig();
    EXPECT(fired1);
    EXPECT(fired2);

    reset();

    con1.disconnect();
    EXPECT(!con1.connected());

    sig();

    EXPECT(!fired1);
    EXPECT(fired2);

    fired2 = false;
    con2.disconnect();

    sig();
    EXPECT(!fired1);
    EXPECT(!fired2);

    reset();
    sig.connect(slot1);
    sig.connect(slot2);
    sig();
    EXPECT(fired1);
    EXPECT(fired2);

    sig.disconnect();  // disconnect all connections
    reset();
    sig();
    EXPECT(!fired1);
    EXPECT(!fired2);
  }
  TEST_CASE_E()

  TEST_CASE_B(connection_disconnect_reconnect) {
    Signal<int> sigInt;
    auto notifiedInt = 0;
    auto con = sigInt.connect([&notifiedInt](int val) { notifiedInt = val; });

    sigInt.notify(10);
    EXPECT(notifiedInt == 10);
    con.disconnect();
    sigInt.notify(100);
    EXPECT(notifiedInt == 10);
    con.reconnect();
    sigInt.notify(-1);
    EXPECT(notifiedInt == -1);
  }
  TEST_CASE_E()
}

void multi_connection_multi_thread_test() {
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

  TEST_CASE_B(multi_connection_multi_thread) {
    auto comp1 = Component::create("thread1");
    AsyncComponent comp2 = Component::create("thread2");

    comp2.launch([&] {
      dataReadySignal.connect(onDataReady1, this_component::getExecutor());
      dataReadySignal.connect(onDataReady2, this_component::getExecutor());
      receiverReadySignal();
    });

    receiverReadySignal.connect(emitDataReadySignal, comp1->getExecutor());

    outOfDataSignal.connect(
        [&] {
          this_component::stop();
          receiverStopSignal.notify();
        },
        comp2->getExecutor());

    receiverStopSignal.connect([] { this_component::stop(); },
                               comp1->getExecutor());

    comp1->run();

    EXPECT(rd1->size() > 0);
    EXPECT(*rd1 == *rd2);
  }

  TEST_CASE_E()
}

void scoped_connection_test() {
  Signal<int> sig;
  int normalInt = 0;
  int scopedInt = 0;
  int scopedInt1 = 0;

  TEST_CASE_B(scoped_connection) {
    {
      auto con = sig.connect([&normalInt](int val) { normalInt = val; });

      ScopedConnection scon =
          sig.connect([&scopedInt](int i) { scopedInt = i; });
      {
        ScopedConnection scon1 =
            sig.connect([&scopedInt1](int x) { scopedInt1 = x; });
        sig.notify(10);

        EXPECT(scopedInt == 10 && scopedInt1 == 10 && normalInt == 10);
      }

      // out of scope, then scopedInt must still be 10 after emit signal again
      sig.notify(100);
      //      EXPECT(scopedInt == 100 && scopedInt1 == 10 && normalInt == 100);
      EXPECT(scopedInt == 100);
      EXPECT(normalInt == 100);
      EXPECT(scopedInt1 == 10);
    }

    sig.notify(1000);
    EXPECT(scopedInt == 100 && scopedInt1 == 10 && normalInt == 1000);

    sig.disconnect();
    sig.notify(10000);
    EXPECT(normalInt == 1000);
  }
  TEST_CASE_E()
}

void connection_aware_slot_test() {
  Signal<int> sigInt;
  int testVal = 0;
  int testVal1 = 0;
  TEST_CASE_B(connection_aware_slot) {
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
    EXPECT(testVal == 1000 && testVal1 == 1000);
    EXPECT(!con.connected());

    sigInt(-1);
    EXPECT(testVal == 1000 && testVal1 == 1000);
  }
  TEST_CASE_E()
}

void trackable_slot_test() {
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

  TEST_CASE_B(trackable_slot) {
    sig(1000);
    EXPECT(ii == 1000);

    pi.reset();
    // after pi was destroyed, it should nolonger receied signal
    sig(3000);
    EXPECT(ii == 1000);
  }
  TEST_CASE_E()
}

void observable_single_state_test() {
  Observable<int> state;
  Observable<vector<int>> vec;

  int observedState = 0;
  auto pStr = make_shared<string>("hello world");
  state.connect(
      pStr, [p = pStr.get()](int state) { *p = to_string(state); },
      maf::util::directExecutor());

  state.connect([&](int val) { observedState = val; });

  TEST_CASE_B(observable_state_pod) {
    state = 1;
    EXPECT(observedState == 1 && state.get() == 1);

    state = 2;
    EXPECT(observedState == 2 && state.get() == 2);

    EXPECT(*pStr == "2");

    pStr.reset();
    // must not crash here
    state = 3;
    EXPECT(observedState == 3 && state.get() == 3);
  }
  TEST_CASE_E(observable_state_pod)

  TEST_CASE_B(observable_state_class) {
    std::vector<int> observedVector;
    int notifyCount = 0;
    vec.connect([&](auto vec) {
      ++notifyCount;
      observedVector = move(vec);
    });

    vec.mut()->assign(3, 10);
    EXPECT((observedVector == vector<int>{10, 10, 10}) && notifyCount == 2);
    static_cast<void>(vec->size());  // call immutable methods won't notify
    EXPECT((observedVector == vector<int>{10, 10, 10}) && notifyCount == 2);
    {
      auto mvec = vec.mut();
      mvec->clear();
      mvec->push_back(100);
      mvec->insert(mvec->begin(), 1000);
    }

    EXPECT((observedVector == vector<int>{1000, 100}));
  }
  TEST_CASE_E()
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

  TEST_CASE_B(arithmetic_observable_test_impl) {
    state = 3;
    ++state;
    EXPECT(observedState == 4 && state.get() == 4);
    state++;
    EXPECT(observedState == 5 && state.get() == 5);
    state += 1;
    EXPECT(observedState == 6 && state.get() == 6);
    state -= 1;
    EXPECT(observedState == 5 && state.get() == 5);
    state *= 2;
    EXPECT(observedState == 10 && state.get() == 10);
    state /= 2;
    EXPECT(observedState == 5 && state.get() == 5);
    state <<= 2;
    EXPECT(observedState == 5 << 2 && state.get() == 5 << 2);
    state >>= 2;
    EXPECT(observedState == 5 && state.get() == 5);

    state = 20;
    // won't notify
    auto newVal = state + 10;
    EXPECT(state.get() == 20 && observedState == 20 &&
           newVal == observedState + 10);
    newVal = state - 10;
    EXPECT(state.get() == 20 && observedState == 20 &&
           newVal == observedState - 10);
    newVal = state * 10;
    EXPECT(state.get() == 20 && observedState == 20 &&
           newVal == observedState * 10);
    newVal = state / 10;
    EXPECT(state.get() == 20 && observedState == 20 &&
           newVal == observedState / 10);
    newVal = state << 2;
    EXPECT(state.get() == 20 && observedState == 20 &&
           newVal == observedState << 2);
    newVal = state >> 2;
    EXPECT(state.get() == 20 && observedState == 20 &&
           newVal == observedState >> 2);
  }
  TEST_CASE_E()
}

void arithmetic_observable_test() {
  arithmetic_observable_test_impl<Observable<int>>();
  arithmetic_observable_test_impl<ObservableST<int>>();
  arithmetic_observable_test_impl<SCObservableST<int>>();
  arithmetic_observable_test_impl<SCObservable<int>>();
}

void observable_multi_state_test() {
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

  TEST_CASE_B(observable_state) {
    state.set(1, "1");
    EXPECT(iVal == 1 && state.get<0>() == 1 && sVal == "1" &&
           state.get<1>() == "1");

    resetExpected();

    state.set(1, "1");

    EXPECT(iVal == -1 && state.get<0>() == 1 && sVal == "" &&
           state.get<1>() == "1");

    resetExpected();
    state.set(2, "1");

    EXPECT(iVal == 2 && state.get<0>() == 2 && sVal == "1" &&
           state.get<1>() == "1");

    resetExpected();
    state.set(2, "2");

    EXPECT(iVal == 2 && state.get<0>() == 2 && sVal == "2" &&
           state.get<1>() == "2");

    state.mut<1>()->push_back('3');
    state.mut<0>()++;
    EXPECT(iVal == 3 && sVal == "23");

    state.mut<0>() += 2;
    EXPECT(iVal == 5);

    state.mut<0>() *= 2;
    EXPECT(iVal == 10);
  }
  TEST_CASE_E()

  TEST_CASE_B(observable_multi_state_concurrent_set) {
    Observable<string, bool, int> sb;
    int notifyCount = 0;
    sb.connect([&](const string&, bool, int) { ++notifyCount; });

    {
      auto msb = sb.mut();
      msb.get<0>().push_back('1');
      msb.get<1>() = false;
      msb.get<1>() = true;
      msb.get<1>() = false;
      msb.get<1>() = true;
      msb.get<2>() += 1;
      msb.get<2>() *= 100;
    }

    EXPECT(notifyCount == 2);
  }
  TEST_CASE_E(observable_multi_state_concurrent_set)
}

void compound_observable_test() {
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

  TEST_CASE_B(compound_observable) {
    pressingKey = "ctrl";
    mouseStatus = MiddlePressed;
    EXPECT(jump == false && notified == 3);

    mouseStatus = LeftReleased;

    EXPECT(jump == false && notified == 4);
    mouseStatus = LeftPressed;
    EXPECT(jump == true && notified == 5);
    pressingKey = "q";
    EXPECT(jump == false && notified == 6);
  }
  TEST_CASE_E()
}

void performance_test() {
  Signal<string> stringSignal;
  const string* s1 = nullptr;
  const string* s2 = nullptr;
  stringSignal.connect([&s1](const string& s) { s1 = &s; });
  stringSignal.connect([&s2](const string& s) { s2 = &s; });

  TEST_CASE_B(copy_constructor_test) {
    string originValue = "helloworld";
    stringSignal(originValue);
    EXPECT(&originValue == s1 && s1 == s2);
  }
  TEST_CASE_E()
}

int main() {
  maf::test::init_test_cases();
  single_connection_single_thread_Test();
  multiconnection_single_thread_test();
  multi_connection_multi_thread_test();
  scoped_connection_test();
  connection_aware_slot_test();
  trackable_slot_test();
  observable_single_state_test();
  arithmetic_observable_test();
  observable_multi_state_test();
  compound_observable_test();
  performance_test();
  return 0;
}
