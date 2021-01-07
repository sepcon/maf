#include <maf/messaging/Component.h>
#include <maf/messaging/SignalTimer.h>
#include <maf/messaging/Timer.h>
#include <maf/utils/TimeMeasurement.h>

#include "test.h"

using namespace std;
using namespace chrono;
using namespace maf::util;
using namespace maf::signal_slots;
using namespace maf::messaging;

template <class Iterable>
void print(const Iterable& c) {
  for (const auto& e : c) {
    cout << e << ", ";
  }
  cout << "\n";
}

struct stop_request_signal {};
struct stop_msg {};

struct TimerCreater {
  vector<shared_ptr<Timer>> timers;
  Timer controllerTimer{true};
  vector<int> timersHitCount;
  int runningTime = 100;
  int eachTimerTimeout = 20;

  TimerCreater(int totalTimer = 20, int runningTime = 100,
               int eachTimerTimeout = 20) {
    this->runningTime = runningTime;
    this->eachTimerTimeout = eachTimerTimeout;
    timersHitCount.resize(totalTimer);
    for (int i = 0; i < totalTimer; ++i) {
      timers.emplace_back(make_shared<Timer>(true));
      timersHitCount[i] = 0;
    }
  }

  void start() {
    for (size_t i = 0; i < timers.size(); ++i) {
      timers[i]->start(eachTimerTimeout, [i, this] { timersHitCount[i]++; });
    }

    controllerTimer.start(runningTime, [start = system_clock::now()] {
      cout << "total execution time of controller timer = "
           << duration_cast<milliseconds>(system_clock::now() - start).count()
           << endl;
      this_component::post(stop_msg{});
    });
  }
};

class TimerRestartTester {
 public:
  Timer counter{true};
  Timer restartee{true};
  int counterHits = 0;
  int restarteeHits = 0;
  void start() {
    auto comp = Component::create();
    comp->connect<int>([this](auto) { this->start_(); });
    comp->connect<stop_msg>([] { this_component::stop(); });

    comp->post(1);
    comp->run();
  }

  void start_() {
    int counterInterval = 1;
    int restarteInterval = 3;
    int maxCounterHits = 9;

    counter.setCyclic();
    restartee.setCyclic();
    restartee.start(restarteInterval,
                    [maxCounterHits, startTime = system_clock::now()] {
                      TEST_CASE_B(test_service) {
                        auto elapsed = system_clock::now() - startTime;
                        EXPECT(elapsed > milliseconds{maxCounterHits});
                      }
                      TEST_CASE_E(test_service)
                      this_component::post(stop_msg{});
                    });

    counter.start(counterInterval, [this, maxCounterHits] {
      ++counterHits;
      if (counterHits % 2 == 0) {
        restartee.restart();
      } else if (counterHits == maxCounterHits) {
        counter.stop();
      }
    });
  }
};

void restartTimerTest() {
  TimerRestartTester tr;
  tr.start();
}

void multiTimersTest() {
  auto comp = Component::create();

  const auto totalTimers = 100;
  const auto testTime = 16;
  const auto eachTimerDuration = 4;
  TimerCreater tc{totalTimers, testTime, eachTimerDuration};
  auto expectation = vector<int>(totalTimers, testTime / eachTimerDuration);
  comp->connect<int>([](int x) { std::cout << "got x = " << x << std::endl; });

  comp->connect<stop_msg>([](auto) { this_component::stop(); });

  comp->run([&tc] { tc.start(); });

  //  print(tc.timersHitCount);
  TEST_CASE_B(timer_expiration) { EXPECT(tc.timersHitCount >= expectation); }
  TEST_CASE_E(timer_expiration)
}

void singleShotTest() {
  maf::util::TimeMeasurement tm;
  long long elapsedMicroseconds = 0;
  Component::create()->run([&]() mutable {
    tm.restart();
    Timer::timeoutAfter(1, [&] {
      elapsedMicroseconds = tm.elapsedTime().count();
      this_component::stop();
    });
  });

  cout << elapsedMicroseconds << endl;
  TEST_CASE_B(signal_timer) {
    EXPECT(elapsedMicroseconds > 900 && elapsedMicroseconds <= 3000);
  }
  TEST_CASE_E()
}

int main() {
  cout.sync_with_stdio(false);
  maf::test::init_test_cases();
  multiTimersTest();
  restartTimerTest();
  singleShotTest();
  return 0;
}
