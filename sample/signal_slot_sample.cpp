#include <maf/Messaging.h>
#include <maf/utils/SignalSlot.h>
#include <maf/utils/StringifyableEnum.h>

#include <iostream>

using namespace maf::util;
using namespace maf::messaging;
using namespace std;

class SignalTimer {
 public:
  using TimeoutSignal = Signal<>;

  void start(long long interval) {
    impl_.start(interval, [this] { timeoutSignal().notify(); });
  }

  void restart() { impl_.restart(); }

  void stop() { impl_.stop(); }

  bool running() const { return impl_.running(); }

  void setCyclic(bool yes = true) { impl_.setCyclic(yes); }

  TimeoutSignal& timeoutSignal() { return timeoutSignal_; }

 private:
  TimeoutSignal timeoutSignal_;
  Timer impl_;
};

int main() {
  auto runner = Component::create();
  SignalTimer timer;
  SignalTimer::TimeoutSignal::ConnectionPtr counterConnection;
  int counter = 0;

  timer.setCyclic();
  timer.timeoutSignal().connect([&counterConnection] {
    static int num = 0;
    cout << (num++ % 2 == 0 ? "tik" : "tak") << endl;
    if (num == 5) {
      counterConnection->reconnect();
    }
  });

  timer.timeoutSignal().connect(
      [&counter,
       &counterConnection](SignalTimer::TimeoutSignal::ConnectionPtr con) {
        cout << "current counter is " << counter << endl;
        counterConnection = con;
        if (++counter == 3) {
          con->disconnect();

        } else if (counter == 10) {
          this_component::stop();
        }
      });

  runner->run([&timer] { timer.start(1000); });
}
