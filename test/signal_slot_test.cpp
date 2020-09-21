
#include <maf/Messaging.h>
#include <maf/utils/DirectExecutor.h>
#include <maf/utils/SignalSlot.h>

#include <map>
#include <thread>

#include "test.h"

using namespace maf;
using namespace maf::messaging;
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
    EXPECT(!con2.connected())  // only one slot can connect

    sig();
    EXPECT(fired)
    fired = false;
    sig.disconnect();
    sig();

    auto con = sig.connect(slot1);
    sig();
    EXPECT(fired)
    con.disconnect();
    fired = false;
    sig();
    EXPECT(!fired)
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
    EXPECT(con1.connected())
    EXPECT(con2.connected())

    sig();
    EXPECT(fired1)
    EXPECT(fired2)

    reset();

    con1.disconnect();
    EXPECT(!con1.connected())

    sig();

    EXPECT(!fired1)
    EXPECT(fired2)

    fired2 = false;
    con2.disconnect();

    sig();
    EXPECT(!fired1)
    EXPECT(!fired2)

    reset();
    sig.connect(slot1);
    sig.connect(slot2);
    sig();
    EXPECT(fired1)
    EXPECT(fired2)

    sig.disconnect();  // disconnect all connections
    reset();
    sig();
    EXPECT(!fired1)
    EXPECT(!fired2)
  }
  TEST_CASE_E()

  TEST_CASE_B(connection_disconnect_reconnect) {
    Signal<int> sigInt;
    auto notifiedInt = 0;
    auto con = sigInt.connect([&notifiedInt](int val) { notifiedInt = val; });

    sigInt.notify(10);
    EXPECT(notifiedInt == 10)
    con.disconnect();
    sigInt.notify(100);
    EXPECT(notifiedInt == 10)
    con.reconnect();
    sigInt.notify(-1);
    EXPECT(notifiedInt == -1)
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
    cout << "Received receiverReadySignal, then send data to receiver "
            "thread"
         << endl;
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
          cout << "Received outOfDataSignal, then theres no more job to do, "
                  "quit!"
               << endl;
          this_component::stop();
          receiverStopSignal.notify();
        },
        comp2->getExecutor());

    receiverStopSignal.connect(
        [] {
          cout << "Received receiverStopSignal, then sender quit, " << endl;
          this_component::stop();
        },
        comp1->getExecutor());

    comp1->run();

    EXPECT(*rd1 == *rd2)
  }

  TEST_CASE_E()
}

void scoped_connection_test() {
  using IntSignal = Signal<int>;
  using SCon = IntSignal::ScopedConnection;

  IntSignal sig;
  int normalInt = 0;
  int scopedInt = 0;
  int scopedInt1 = 0;

  TEST_CASE_B(scoped_connection) {
    {
      auto con = sig.connect([&normalInt](int val) { normalInt = val; });

      SCon scon = sig.connect([&scopedInt](int i) { scopedInt = i; });
      {
        SCon scon1 = sig.connect([&scopedInt1](int x) { scopedInt1 = x; });
        sig.notify(10);

        EXPECT(scopedInt == 10 && scopedInt1 == 10 && normalInt == 10)
      }

      // out of scope, then scopedInt must still be 10 after emit signal again
      sig.notify(100);
      //      EXPECT(scopedInt == 100 && scopedInt1 == 10 && normalInt == 100)
      EXPECT(scopedInt == 100)
      EXPECT(normalInt == 100)
      EXPECT(scopedInt1 == 10)
    }

    sig.notify(1000);
    EXPECT(scopedInt == 100 && scopedInt1 == 10 && normalInt == 1000)

    sig.disconnect();
    sig.notify(10000);
    EXPECT(normalInt == 1000)
  }
  TEST_CASE_E()
}

void connection_aware_slot_test() {
  Signal<int> sigInt;
  int testVal = 0;
  TEST_CASE_B(connection_aware_slot) {
    sigInt.connect(
        [&testVal](const Signal<int>::ConnectionPtr& con, int val) {
          con->disconnect();
          testVal = val;
        },
        directExecutor());
    sigInt(1000);
    EXPECT(testVal == 1000)
    sigInt(-1);
    EXPECT(testVal == 1000)
  }
  TEST_CASE_E()
}

void performance_test() {
  Signal<const string&> stringSignal;
  const string* s1 = nullptr;
  const string* s2 = nullptr;
  stringSignal.connect([&s1](const string& s) { s1 = &s; });
  stringSignal.connect([&s2](const string& s) { s2 = &s; });

  TEST_CASE_B(copy_constructor_test) {
    string originValue = "helloworld";
    stringSignal(originValue);
    EXPECT(&originValue == s1 && s1 == s2)
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
  performance_test();
}
