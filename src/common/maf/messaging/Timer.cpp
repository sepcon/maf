#include <maf/logging/Logger.h>
#include <maf/messaging/Processor.h>
#include <maf/messaging/Timer.h>

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
  void restart() {
    running = true;
    deadline = Clock::now() + duration;
  }
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
  enum class State : char { NoTimer, HaveTimer, Waiting };
  Heap records_;
  State state_ = State::NoTimer;

  void cleanup();
  void refresh();
  void checkAllTimers();
  void start(TimerDataPtr record);
  void stop(TimerDataPtr record);
  void onTimerModified();
  void onShortestTimerExpired(const TimerDataPtr& record);
  void restart(const TimerDataPtr& r);
  void add(TimerDataPtr newRecord);
  bool remove(TimerDataPtr tm);
  TimerDataPtr getShortestTimer();
  TimerDataPtr removeShortestTimer();
  void updateShortestTimer();
  void interruptCurrentTimer(const ProcessorInstance& comp);
  bool checkRecordListEmpty();

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
                     TimeOutCallback&& callback) {
  tm->reset(move(callback), interval, tm->cyclic);
  mgr().start(tm);
}

Timer::Timer(bool cyclic) : d_{new TimerData} { d_->cyclic = cyclic; }

Timer::~Timer() { stop(); }
void Timer::start(long long milliseconds, TimeOutCallback callback) {
  start(std::chrono::milliseconds{milliseconds}, std::move(callback));
}
void Timer::start(std::chrono::milliseconds interval,
                  TimeOutCallback callback) {
  assert(callback);
  runTimer(d_, interval, move(callback));
}

void Timer::start(long long milliseconds, Timer::TimeOutCallback callback,
                  const ProcessorInstance& comp) {
  start(std::chrono::milliseconds{milliseconds}, move(callback), comp);
}

void Timer::start(milliseconds milliseconds, Timer::TimeOutCallback callback,
                  const ProcessorInstance& comp) {
  comp->executeAsync(
      [timerData = d_, callback{move(callback)}, milliseconds]() mutable {
        runTimer(timerData, milliseconds, move(callback));
      });
}

void Timer::restart() { mgr().restart(d_); }

void Timer::stop() { mgr().stop(d_); }

void Timer::stop(const ProcessorInstance& comp) {
  assert(comp);
  comp->executeAsync([timerData{d_}] { mgr().stop(timerData); });
}

bool Timer::running() const { return d_->running; }

void Timer::setCyclic(bool cyclic) {
  if (cyclic != d_->cyclic) {
    d_->cyclic = cyclic;
  }
}

void Timer::timeoutAfter(long long ms, Timer::TimeOutCallback callback) {
  timeoutAfter(milliseconds{ms}, move(callback));
}

void Timer::timeoutAfter(milliseconds milliseconds,
                         Timer::TimeOutCallback callback) {
  auto tm = make_shared<TimerData>();
  runTimer(tm, milliseconds, move(callback));
}

void Timer::timeoutAfter(long long ms, Timer::TimeOutCallback callback,
                         const ProcessorInstance& comp) {
  timeoutAfter(milliseconds{ms}, move(callback), comp);
}

void Timer::timeoutAfter(milliseconds milliseconds,
                         Timer::TimeOutCallback callback,
                         const ProcessorInstance& comp) {
  comp->executeAsync([callback{move(callback)}, milliseconds]() mutable {
    auto tm = make_shared<TimerData>();
    runTimer(tm, milliseconds, move(callback));
  });
}

void TimerMgr::cleanup() {
  records_.clear();
  state_ = State::NoTimer;
}

void TimerMgr::refresh() { make_heap(begin(), end(), timerGreater); }

void TimerMgr::checkAllTimers() {
  if (state_ == State::Waiting) {
    return;
  }
  auto comp = this_processor::instance();
  while (auto timer = getShortestTimer()) {
    if (!timer->expired()) {
      state_ = State::Waiting;
      comp->runOnceUntil(timer->deadline);
      state_ = State::HaveTimer;
    }

    if (comp->stopped()) {
      break;
    }

    if (!timer->expired()) {
      continue;
    }

    state_ = State::HaveTimer;
    // shortest timer might be stopped while waiting
    if (timer->running) {
      onShortestTimerExpired(timer);
    }
  }
}

void TimerMgr::start(TimerDataPtr record) {
  if (!record->running) {
    record->running = true;
    add(record);
  } else {
    refresh();
  }
  onTimerModified();
}

void TimerMgr::stop(TimerDataPtr record) {
  if (record->running) {
    record->running = false;
    if (auto removedShortest = remove(record);
        removedShortest && !records_.empty()) {
      interruptCurrentTimer(this_processor::instance());
    }
  }
}

void TimerMgr::onTimerModified() {
  auto thisProcessorInstance = this_processor::instance();
  assert(thisProcessorInstance &&
         "Timer must be triggered in thread of a mesasging::Processor")
      thisProcessorInstance->executeAsync([this] { this->checkAllTimers(); });
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
  onTimerModified();
}

void TimerMgr::add(TimerDataPtr newRecord) {
  using namespace std;
  records_.push_back(move(newRecord));
  push_heap(begin(), end(), timerGreater);
}

bool TimerMgr::remove(TimerDataPtr tm) {
  auto removedShortestOne = false;
  if (!records_.empty()) {
    if (records_.front() == tm) {
      removeShortestTimer();
      removedShortestOne = true;
    } else if (auto it = std::remove(begin(), end(), tm); it != end()) {
      records_.erase(it);
      if (!checkRecordListEmpty()) {
        refresh();
      }
    }
  }
  return removedShortestOne;
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
  checkRecordListEmpty();
  return tm;
}

void TimerMgr::updateShortestTimer() {
  records_.front()->updateNextDeadline();
  pop_heap(begin(), end(), timerGreater);
  push_heap(begin(), end(), timerGreater);
}

void TimerMgr::interruptCurrentTimer(const ProcessorInstance& comp) {
  if (state_ == State::Waiting && comp) {
    comp->executeAsync([] {
      // an empty callback just to wakeup runOnceUntil wait
    });
  }
}

bool TimerMgr::checkRecordListEmpty() {
  if (records_.empty()) {
    state_ = State::NoTimer;
    return true;
  }
  return false;
}

}  // namespace messaging
}  // namespace maf
