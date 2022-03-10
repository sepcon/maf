#include <maf/messaging/ProcessorEx.h>
#include <maf/messaging/Timer.h>

#include <iostream>

using namespace maf::messaging;
using namespace std;
using namespace std::chrono;

struct next_second_hit {
  seconds current_seconds;
};

struct alarm_message {
  string msg;
};

struct Alarm {
  int alarmTime = 12;
  seconds totalAlarmInterval = 1s;
  milliseconds elapsedAlarm = 0ms;
  milliseconds eachAlarmInterval = 100ms;
  seconds currentSeconds = 0s;
  Timer clock;
  Timer alarmTimer;

  void start() {
    auto runner = Processor::create();
    runner->connect<next_second_hit>([this](next_second_hit h) {
      if (h.current_seconds.count() % alarmTime == 0) {
        alarmTimer.setCyclic();

        alarmTimer.start(eachAlarmInterval, [] {
          this_processor::post<alarm_message>("wake up!");
        });
      }
    });

    runner->connect<alarm_message>([this](const alarm_message& msg) {
      cout << "[ALARM]: " << msg.msg << endl;
      elapsedAlarm += eachAlarmInterval;
      if (elapsedAlarm == totalAlarmInterval) {
        elapsedAlarm = 0ms;
        alarmTimer.stop();
      }
    });

    runner->run([this] {
      clock.setCyclic();
      clock.start(1s, [this] {
        cout << (currentSeconds.count() % 2 == 0 ? "tik" : "tak") << endl;
        this_processor::post<next_second_hit>(++currentSeconds);
      });
    });
  }
};

int main() { Alarm{}.start(); }
