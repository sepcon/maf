#include <maf/logging/Logger.h>
#include <maf/messaging/Component.h>
#include <maf/messaging/Timer.h>
#include <maf/utils/CallOnExit.h>

#include <algorithm>
#include <cassert>
#include <memory>
#include <thread>
#include <vector>

namespace maf {
namespace messaging {

struct TimerData;
struct TimerInterrupt;
struct TimerMgr;
using namespace std::chrono;
using Clock = system_clock;
using DeadLine = ExecutionDeadline;
using TimeOutCallback = Timer::TimeOutCallback;
using std::make_heap;
using std::make_shared;
using std::move;
using std::pop_heap;
using std::push_heap;
using std::shared_ptr;
using TimerDataPtr = shared_ptr<TimerData>;
using util::ExecutorIFPtr;

struct TimerInterrupt {};

struct TimerData {
  TimeOutCallback callback;
  DeadLine deadline = Clock::now();
  ExecutionTimeout duration;
  bool cyclic = false;
  bool running = false;

  TimerData() = default;
  TimerData(TimeOutCallback cb, ExecutionTimeout interval, bool cc = false)
      : callback(move(cb)),
        deadline(Clock::now() + interval),
        duration(interval),
        cyclic(cc) {}
  void updateNextDeadline() { deadline += duration; }
  void restart() { deadline = Clock::now() + duration; }
  bool expired() const { return deadline <= Clock::now(); }
  void onExpired() { callback(); }
  void reset(TimeOutCallback&& cb, ExecutionTimeout d, bool cc = false) {
    callback = move(cb);
    deadline = Clock::now() + d;
    duration = d;
    cyclic = cc;
  }
};

static bool timerGreater(const TimerDataPtr& t1, const TimerDataPtr& t2) {
  return t1->deadline > t2->deadline;
}

struct TimerMgr {
  using Heap = std::vector<TimerDataPtr>;
  Heap records_;
  std::thread::id mythreadID;

  void cleanup();
  void refresh();
  void runAllTimers();
  void start(TimerDataPtr record);
  void onTimerModified();
  void onShortestTimerExpired(const TimerDataPtr& record);
  void restart(const TimerDataPtr& r);
  void add(TimerDataPtr newRecord);
  bool runningOnThisThread() const;
  void setRunningOnThisThread(bool yes = true);
  void remove(TimerDataPtr tm);
  TimerDataPtr getShortestTimer();
  TimerDataPtr removeShortestTimer();
  void updateShortestTimer();
  void interruptCurrentTimer(const ComponentInstance& comp);

  decltype(auto) begin() { return std::begin(records_); }
  decltype(auto) end() { return std::end(records_); }
  decltype(auto) begin() const { return std::begin(records_); }
  decltype(auto) end() const { return std::end(records_); }
};

static TimerMgr& mgr() {
  static thread_local TimerMgr _;
  return _;
}

static void runTimer(const TimerDataPtr& tm, milliseconds interval,
                     TimeOutCallback&& callback, ExecutorIFPtr&& executor) {
  if (auto comp = this_component::instance()) {
    if (executor) {
      tm->reset([callback = move(callback),
                 executor = move(executor)] { executor->execute(callback); },
                interval, tm->cyclic);
    } else {
      tm->reset(move(callback), interval, tm->cyclic);
    }
    mgr().start(tm);
  }
}

Timer::Timer(bool cyclic) : d_{new TimerData} { d_->cyclic = cyclic; }

Timer::~Timer() { stop(); }
void Timer::start(long long milliseconds, TimeOutCallback callback,
                  ExecutorIFPtr executor) {
  start(std::chrono::milliseconds{milliseconds}, std::move(callback),
        std::move(executor));
}
void Timer::start(std::chrono::milliseconds interval, TimeOutCallback callback,
                  ExecutorIFPtr executor) {
  if (!callback) {
    MAF_LOGGER_ERROR("[TimerImpl]: Please specify not null callback");
  } else {
    runTimer(d_, interval, move(callback), move(executor));
  }
}

void Timer::restart() {
  if (auto comp = this_component::instance()) {
    d_->running = true;
    mgr().restart(d_);
    mgr().interruptCurrentTimer(comp);
  }
}

void Timer::stop() {
  if (d_->running) {
    d_->running = false;
    if (auto comp = this_component::instance()) {
      mgr().remove(d_);
      mgr().interruptCurrentTimer(comp);
    }
  }
}

bool Timer::running() const { return d_->running; }

void Timer::setCyclic(bool cyclic) {
  if (cyclic != d_->cyclic) {
    d_->cyclic = cyclic;
  }
}

void Timer::timeoutAfter(long long ms, Timer::TimeOutCallback callback,
                         Timer::ExecutorIFPtr executor) {
  timeoutAfter(milliseconds{ms}, move(callback), move(executor));
}

void Timer::timeoutAfter(milliseconds milliseconds,
                         Timer::TimeOutCallback callback,
                         Timer::ExecutorIFPtr executor) {
  auto tm = make_shared<TimerData>();
  runTimer(tm, milliseconds, move(callback), move(executor));
}

void TimerMgr::cleanup() {
  records_.clear();
  setRunningOnThisThread(false);
}

void TimerMgr::refresh() {
  using namespace std;
  make_heap(begin(), end(), timerGreater);
}

void TimerMgr::runAllTimers() {
  util::CallOnExit onExit = [this] { cleanup(); };
  setRunningOnThisThread();
  auto comp = this_component::instance();
  do {
    auto timer = getShortestTimer();
    if (!timer) {
      break;
    }

    try {
      if (!timer->expired()) {
        comp->runUntil(timer->deadline);
        if (!timer->expired()) {
          if (!comp->stopped()) {
            continue;
          } else {
            break;
		  }
        }
      }

      if (!comp->stopped()) {
        onShortestTimerExpired(timer);
      } else {
        break;
      }
    } catch (TimerInterrupt) {
      continue;
    }
  } while (true);
}

void TimerMgr::start(TimerDataPtr record) {
  if (!record->running) {
    record->running = true;
    add(record);
  } else {
    refresh();
  }
  // triggerTimer should be executed async to avoid blocking the current
  // thread
  this_component::instance()->execute([this] { this->onTimerModified(); });
}

void TimerMgr::onTimerModified() {
  if (runningOnThisThread()) {
    interruptCurrentTimer(this_component::instance());
  } else {
    runAllTimers();
  }
}

void TimerMgr::onShortestTimerExpired(const TimerDataPtr& record) {
  if (record->cyclic) {
    updateShortestTimer();
  } else {
    record->running = false;
    removeShortestTimer();
  }
  record->onExpired();
}

void TimerMgr::restart(const TimerDataPtr& r) {
  r->restart();
  if (r == records_.front()) {
    pop_heap(begin(), end(), timerGreater);
    push_heap(begin(), end(), timerGreater);
  } else {
    refresh();
  }
}

void TimerMgr::add(TimerDataPtr newRecord) {
  using namespace std;
  records_.push_back(move(newRecord));
  push_heap(begin(), end(), timerGreater);
}

bool TimerMgr::runningOnThisThread() const {
  return std::this_thread::get_id() == mythreadID;
}

void TimerMgr::setRunningOnThisThread(bool yes) {
  if (yes) {
    mythreadID = std::this_thread::get_id();
  } else {
    mythreadID = {};
  }
}

void TimerMgr::remove(TimerDataPtr tm) {
  using namespace std;
  records_.erase(std::remove(begin(), end(), tm));
  make_heap(begin(), end(), timerGreater);
}

TimerDataPtr TimerMgr::getShortestTimer() {
  if (!records_.empty()) {
    return records_.front();
  }
  return {};
}

TimerDataPtr TimerMgr::removeShortestTimer() {
  using namespace std;
  pop_heap(begin(), end(), timerGreater);
  auto tm = move(records_.back());
  records_.pop_back();
  return tm;
}

void TimerMgr::updateShortestTimer() {
  records_.front()->updateNextDeadline();
  pop_heap(begin(), end(), timerGreater);
  push_heap(begin(), end(), timerGreater);
}

void TimerMgr::interruptCurrentTimer(const ComponentInstance& comp) {
  comp->execute([this] {
    if (runningOnThisThread()) {
      throw TimerInterrupt{};
    }
  });
}

}  // namespace messaging
}  // namespace maf
