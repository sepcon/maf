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
using TimeoutCallback = Timer::TimeOutCallback;
using std::make_heap;
using std::make_shared;
using std::move;
using std::pop_heap;
using std::push_heap;
using std::shared_ptr;
using TimerDataPtr = shared_ptr<TimerData>;

struct TimerInterrupt {};

struct TimerData {
  TimeoutCallback callback;
  ExecutorPtr executor;
  DeadLine deadline = Clock::now();
  ExecutionTimeout duration;
  bool cyclic = false;
  bool running = false;

  TimerData() = default;
  TimerData(TimeoutCallback cb, ExecutionTimeout interval, bool cc = false)
      : callback(move(cb)),
        deadline(Clock::now() + interval),
        duration(interval),
        cyclic(cc) {}
  void updateNextDeadline() { deadline += duration; }
  void restart() { deadline = Clock::now() + duration; }
  bool expired() const { return deadline <= Clock::now(); }
  void onExpired() {
    if (executor) {
      executor->execute(callback);
    } else {
      callback();
    }
  }
  void reset(TimeoutCallback&& cb, ExecutorPtr&& exc, ExecutionTimeout d,
             bool cc = false) {
    callback = move(cb);
    executor = move(exc);
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

static TimerMgr& recMgr() {
  static thread_local TimerMgr _;
  return _;
}

Timer::Timer(bool cyclic) : d_{new TimerData} { d_->cyclic = cyclic; }

Timer::~Timer() { stop(); }
void Timer::start(long long milliseconds, TimeOutCallback callback,
                  ExecutorPtr executor) {
  start(std::chrono::milliseconds{milliseconds}, std::move(callback),
        std::move(executor));
}
void Timer::start(std::chrono::milliseconds interval, TimeOutCallback callback,
                  ExecutorPtr executor) {
  if (!callback) {
    MAF_LOGGER_ERROR("[TimerImpl]: Please specify not null callback");
  } else {
    if (auto comp = this_component::instance()) {
      d_->reset(move(callback), move(executor), interval, d_->cyclic);
      recMgr().start(d_);
    }
  }
}

void Timer::restart() {
  if (auto comp = this_component::instance()) {
    if (d_->running) {
      recMgr().restart(d_);
      recMgr().interruptCurrentTimer(comp);
    }
  }
}

void Timer::stop() {
  if (d_->running) {
    d_->running = false;
    if (auto comp = this_component::instance()) {
      recMgr().remove(d_);
      recMgr().interruptCurrentTimer(comp);
    }
  }
}

bool Timer::running() { return d_->running; }

void Timer::setCyclic(bool cyclic) {
  if (cyclic != d_->cyclic) {
    d_->cyclic = cyclic;
  }
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
  do {
    auto timer = getShortestTimer();
    if (!timer) {
      break;
    }

    try {
      auto comp = this_component::instance();
      if (!timer->expired()) {
        comp->runUntil(timer->deadline);
        if (!timer->expired()) {
          continue;
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
