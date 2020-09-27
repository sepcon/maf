#include <maf/Messaging.h>
#include <maf/messaging/SignalTimer.h>
#include <maf/utils/TimeMeasurement.h>

#include <iostream>

using namespace maf::util;
using namespace maf::signal_slots;
using namespace maf::messaging;
using namespace std;

static mutex m;

#define LOG(expr)         \
  {                       \
    lock_guard lock(m);   \
    cout << expr << endl; \
  }


int main() {
  auto tm = TimeMeasurement(
      [](auto elapsed) { LOG("elapsed = " << elapsed.count() << "us"); });

  cout.sync_with_stdio(false);
  auto runner = Component::create();
  AsyncComponent runner2 = Component::create();

  Signal<> stopAllSignal;
  Signal<> firedSignal;

  SignalTimer timer;
  int counter = 0;

  timer.timeoutSignal.connect(ref(firedSignal));

  firedSignal.connect([] { LOG("Timeout on runner"); }, runner->getExecutor());

  stopAllSignal.connect([runner, &runner2, &tm] {
    tm.stop();
    runner->stop();
    runner2->stop();
  });

  timer.setCyclic(true);

  timer.timeoutSignal.connect([] { LOG("Time out on runner2"); },
                              runner->getExecutor());

  timer.timeoutSignal.connect([] { LOG("Time out on runner2"); },
                              runner->getExecutor());

  timer.timeoutSignal.connect([&](auto connectionPtr) {
    if (++counter >= 10) {
      stopAllSignal();
      connectionPtr->disconnect();
    }
  });

  runner2.launch();
  runner->run([&timer] { timer.start(100); });
  runner2.wait();
  LOG("Counter is " << counter);
}
