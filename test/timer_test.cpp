#include <maf/messaging/Component.h>
#include <maf/messaging/Timer.h>

#include "test.h"

using namespace std;
using namespace chrono;
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
    };

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
    comp->onMessage<int>([this](auto) { this->start_(); });
    comp->onMessage<stop_msg>([] { this_component::stop(); });

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
                      MAF_TEST_CASE_BEGIN(test_service) {
                        auto elapsed = system_clock::now() - startTime;
                        MAF_TEST_EXPECT(elapsed > milliseconds{maxCounterHits});
                      }
                      MAF_TEST_CASE_END(test_service)
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
  const auto testTime = 10;
  const auto eachTimerDuration = 1;
  TimerCreater tc{totalTimers, testTime, eachTimerDuration};
  auto expectation = vector<int>(totalTimers, testTime / eachTimerDuration);
  comp->onMessage<int>(
      [](int x) { std::cout << "got x = " << x << std::endl; });

  comp->onMessage<stop_msg>([](auto) { this_component::stop(); });

  comp->run([&tc] { tc.start(); });

//  print(tc.timersHitCount);
  MAF_TEST_CASE_BEGIN(timer_expiration) {
    MAF_TEST_EXPECT(tc.timersHitCount >= expectation);
  }
  MAF_TEST_CASE_END(timer_expiration)
}

int main() {
  cout.sync_with_stdio(false);
  maf::test::init_test_cases();
  multiTimersTest();
  restartTimerTest();
  return 0;
}
