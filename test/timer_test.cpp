#include <maf/messaging/Processor.h>
#include <maf/messaging/SignalTimer.h>
#include <maf/messaging/Timer.h>
#include <maf/utils/TimeMeasurement.h>

#include <iostream>

#define CATCH_CONFIG_MAIN
#include "catch/catch_amalgamated.hpp"

using namespace std;
using namespace chrono;
using namespace maf::util;
using namespace maf::signal_slots;
using namespace maf::messaging;

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
      //      cout << "total execution time of controller timer = "
      //           << duration_cast<milliseconds>(system_clock::now() -
      //           start).count()
      //           << endl;
      this_processor::post(stop_msg{});
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
    auto comp = Processor::create();
    comp->connect<int>([this](auto) { this->start_(); });
    comp->connect<stop_msg>([] { this_processor::stop(); });

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
                      auto elapsed = system_clock::now() - startTime;
                      REQUIRE(elapsed > milliseconds{maxCounterHits});

                      this_processor::post(stop_msg{});
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

TEST_CASE("restartTimer") {
  TimerRestartTester tr;
  tr.start();
}

TEST_CASE("multiTimers") {
  auto comp = Processor::create();

  const auto totalTimers = 100;
  const auto testTime = 16;
  const auto eachTimerDuration = 4;
  TimerCreater tc{totalTimers, testTime, eachTimerDuration};
  auto expectation = vector<int>(totalTimers, testTime / eachTimerDuration);
  comp->connect<int>([](int x) { std::cout << "got x = " << x << std::endl; });

  comp->connect<stop_msg>([](auto) { this_processor::stop(); });

  comp->run([&tc] { tc.start(); });

  REQUIRE(tc.timersHitCount >= expectation);
}

TEST_CASE("singleShot") {
  maf::util::TimeMeasurement tm;
  long long elapsedMicroseconds = 0;
  Processor::create()->run([&]() mutable {
    tm.restart();
    Timer::timeoutAfter(1, [&] {
      elapsedMicroseconds = tm.elapsedTime().count();
      this_processor::stop();
    });
  });

  INFO("Elapsed time = " << elapsedMicroseconds);
  REQUIRE(elapsedMicroseconds > 900);
  // the min resolution in Windows is 15ms, then shouldnt expect it to work
  // correctly with 1ms here
  REQUIRE(elapsedMicroseconds <= 16900);
}

TEST_CASE("messageprocessorCorrectness") {
  auto c = Processor::create();
  const auto totalExecutions = 100000;
  auto executedCount = 0;
  Timer t;
  c->run([&] {
    for (int i = 0; i < totalExecutions; ++i) {
      this_processor::instance()->executeAsync([&] {
        executedCount++;
        if (executedCount == totalExecutions) {
          this_processor::stop();
        }
      });
    }

    t.setCyclic(true);
    t.start(10ms, [] { this_processor::stop(); });
  });

  REQUIRE(executedCount == totalExecutions);
}
