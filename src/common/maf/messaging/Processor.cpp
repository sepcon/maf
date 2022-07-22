#include <maf/SignalSlots.h>
#include <maf/logging/Logger.h>
#include <maf/messaging/Processor.h>
#include <maf/threading/Lockable.h>
#include <maf/threading/Queue.h>
#include <maf/utils/CallOnExit.h>

#include <cassert>
#include <cstring>
#include <forward_list>
#include <future>
#include <map>
#include <string_view>

#include "Router.h"

namespace maf {
namespace messaging {
using namespace std::string_view_literals;

namespace this_processor {
static bool testAndSetThreadLocalInstance(Processor *inst);
static void clearTLInstanceIfSet(bool set);
static thread_local Processor *instance_ = nullptr;
}  // namespace this_processor

using ExecutionUPtr = std::unique_ptr<Execution>;
using Handlers = signal_slots::Signal<const Message &>;
using HandlersPtr = std::shared_ptr<Handlers>;
using PendingExecutions = threading::Queue<ExecutionUPtr>;
using MsgHandlersMap = threading::Lockable<std::map<MessageID, HandlersPtr>>;
using util::CallOnExit;
using SSConnection = signal_slots::Connection;

static inline constexpr auto anonymous_prefix = "[anonymous]."sv;

class AsyncExecutor : public util::ExecutorIF {
  ProcessorRef compref;

 public:
  AsyncExecutor(ProcessorRef &&cr) : compref{std::move(cr)} {}
  bool execute(CallbackType callback) noexcept override {
    if (auto comp = compref.lock()) {
      comp->executeAsync(std::move(callback));
      return true;
    }
    return false;
  }
};

class DefaultExecutor : public util::ExecutorIF {
  ProcessorRef compref;

 public:
  DefaultExecutor(ProcessorRef &&cr) : compref{std::move(cr)} {}
  bool execute(CallbackType callback) noexcept override {
    if (auto comp = compref.lock()) {
      comp->execute(std::move(callback));
      return true;
    }
    return false;
  }
};
class WaitableExecutor : public util::ExecutorIF {
  ProcessorRef compref;

 public:
  WaitableExecutor(ProcessorRef &&cr) : compref{std::move(cr)} {}
  bool execute(CallbackType callback) noexcept override {
    if (auto comp = compref.lock()) {
      try {
        comp->waitableExecute(callback).wait();
        return true;
      } catch (const std::future_error &e) {
        MAF_LOGGER_ERROR("Cauth exception when execute callback: ", e.what());
        return false;
      }
    }
    return false;
  }
};

struct ProcessorDataPrv {
  ProcessorDataPrv(ProcessorID id) : id{std::move(id)} {}
  ProcessorID id;
  PendingExecutions pendingExecutions;
  MsgHandlersMap msgHandlersMap;

  bool addExecution(Execution e) {
    try {
      pendingExecutions.push(std::make_unique<Execution>(move(e)));
      return true;
    } catch (const std::bad_alloc &ba) {
      MAF_LOGGER_ERROR("Queue overflow: ", ba.what());
    }
    return false;
  }

  void processMessage(const Message &msg) {
    HandlersPtr handlers;
    {
      std::lock_guard lock(msgHandlersMap);
      auto itHandler = msgHandlersMap->find(msg.type());
      if (itHandler != msgHandlersMap->end()) {
        handlers = itHandler->second;
      }
    }

    if (handlers) {
      handlers->notify(msg);
    }
  }

  bool msgConnected(const MessageID &msgID) {
    std::lock_guard lock(msgHandlersMap);
    auto it = msgHandlersMap->find(msgID);
    return it != msgHandlersMap->end() && it->second->connected();
  }

  void cleanupUnconnectedMsgHandlers() {
    for (auto it = msgHandlersMap->begin(); it != msgHandlersMap->end();) {
      if (!it->second->connected()) {
        it = msgHandlersMap->erase(it);
      } else {
        ++it;
      }
    }
  }

  void closeAndClearExecutionsQueue() {
    pendingExecutions.close();
    pendingExecutions.clear();
  }
};

static decltype(auto) invoke(const ExecutionUPtr &exc) { return (*exc)(); }

static const ProcessorID &emptyProcessorID() {
  static ProcessorID emptyID;
  return emptyID;
}

static ProcessorID generateAnonymousID() {
  static std::atomic_int counter = 0;
  return anonymous_prefix.data() + std::to_string(++counter);
}

static bool isAnonymous(const ProcessorID &id) {
  return id.length() > anonymous_prefix.length() &&
         std::string_view(id.c_str(), anonymous_prefix.length()) ==
             anonymous_prefix;
}

static void leaveRoutingIfNotAnonymous(ProcessorInstance comp) {
  if (!isAnonymous(comp->id())) {
    Router::instance().removeProcessor(comp);
  }
}

Processor::Processor(ProcessorID id)
    : d_{new ProcessorDataPrv{std::move(id)}} {}

Processor::~Processor() { d_->closeAndClearExecutionsQueue(); }

ProcessorInstance Processor::create(ProcessorID id) {
  auto willJoinRouting = !id.empty();
  if (willJoinRouting) {
    assert(!isAnonymous(id));
    // Make sure the Router::instance is created before all Processor instances
    // that depend on it
    Router::instance();
  } else {
    id = generateAnonymousID();
  }

  auto comp = ProcessorInstance{new Processor{std::move(id)}};

  if (willJoinRouting) {
    if (Router::instance().findProcessor(comp->id())) {
      comp.reset();
    } else {
      Router::instance().addProcessor(comp);
    }
  }
  return comp;
}

ProcessorInstance Processor::findProcessor(const ProcessorID &id) {
  return Router::instance().findProcessor(id);
}

const ProcessorID &Processor::id() const noexcept { return d_->id; }

void Processor::run(ThreadFunction threadInit, ThreadFunction threadDeinit) {
  auto justSet = this_processor::testAndSetThreadLocalInstance(this);
  if (threadInit) {
    threadInit();
  }

  CallOnExit deinit = [justSet, threadDeinit = std::move(threadDeinit)] {
    if (threadDeinit) {
      threadDeinit();
    }
    this_processor::clearTLInstanceIfSet(justSet);
  };

  ExecutionUPtr exc;
  while (d_->pendingExecutions.wait(exc)) {
    invoke(exc);
  }
}

void Processor::runFor(ExecutionTimeout duration) {
  runUntil(std::chrono::system_clock::now() + duration);
}

void Processor::runUntil(ExecutionDeadline deadline) {
  ExecutionUPtr exc;
  auto justSet = this_processor::testAndSetThreadLocalInstance(this);
  CallOnExit deinit = [justSet] {
    this_processor::clearTLInstanceIfSet(justSet);
  };

  while (d_->pendingExecutions.waitUntil(exc, deadline)) {
    invoke(exc);
  }
}

bool Processor::runOnceFor(ExecutionTimeout duration) {
  return runOnceUntil(std::chrono::system_clock::now() + duration);
}

bool Processor::runOnceUntil(ExecutionDeadline deadline) {
  using namespace std::chrono;
  ExecutionUPtr exc;
  auto justSet = this_processor::testAndSetThreadLocalInstance(this);
  CallOnExit deinit = [justSet] {
    this_processor::clearTLInstanceIfSet(justSet);
  };

  if (d_->pendingExecutions.waitUntil(exc, deadline)) {
    invoke(exc);
    return true;
  }

  return false;
}

void Processor::stop() {
  if (!stopped()) {
    d_->closeAndClearExecutionsQueue();
    leaveRoutingIfNotAnonymous(shared_from_this());
  }
}

void Processor::reuse() { d_->pendingExecutions.reOpen(); }

bool Processor::stopped() const { return d_->pendingExecutions.isClosed(); }

bool Processor::post(Message msg) {
  using namespace std;
  if (!stopped()) {
    auto &msgType = msg.type();
    if (d_->msgConnected(msgType)) {
      return executeAsync([msg = move(msg), this] { d_->processMessage(msg); });
    } else {
      MAF_LOGGER_WARN("There's no handler for message ", msgType.name());
    }
  }
  return false;
}

Processor::CompleteSignal Processor::waitablePost(Message msg) {
  using namespace std;
  CompleteSignal doneSignal;
  if (!stopped()) {
    auto &msgType = msg.type();
    if (auto handlers = d_->msgConnected(msgType)) {
      auto msgHandlingTask = make_shared<packaged_task<void()>>(
          [this, msg = move(msg)] { d_->processMessage(msg); });

      doneSignal = CompleteSignal{msgHandlingTask->get_future()};
      if (this_processor::id() != id()) {
        executeAsync([task{move(msgHandlingTask)}] { (*task)(); });
      } else {
        (*msgHandlingTask)();
      }
    } else {
      MAF_LOGGER_WARN("There's no handler for message ", msgType.name());
    }
  }
  return doneSignal;
}

bool Processor::connected(const MessageID &mid) const {
  return d_->msgHandlersMap.atomic()->count(mid) > 0;
}

bool Processor::executeAsync(Execution exec) {
  return !stopped() ? d_->addExecution(move(exec)) : false;
}

bool Processor::execute(Execution exec) {
  using namespace std;
  if (!stopped()) {
    if (this_processor::instance().get() == this) {
      exec();
      return true;
    } else {
      return d_->addExecution(move(exec));
    }
  }
  return false;
}

Processor::CompleteSignal Processor::waitableExecute(Execution exec) {
  using namespace std;
  CompleteSignal doneSignal;
  if (!stopped()) {
    auto task = make_shared<packaged_task<void()>>(move(exec));
    doneSignal = CompleteSignal{task->get_future()};
    if (this_processor::id() != id()) {
      executeAsync([task{move(task)}] { (*task)(); });
    } else {
      (*task)();
    }
  }
  return doneSignal;
}

Processor::Executor Processor::getExecutor() {
  return std::make_shared<DefaultExecutor>(weak_from_this());
}

Processor::Executor Processor::getAsyncExecutor() {
  return std::make_shared<AsyncExecutor>(weak_from_this());
}

Processor::Executor Processor::getBlockingExecutor() {
  return std::make_shared<WaitableExecutor>(weak_from_this());
}

Execution Processor::willExecuteOnThis(Execution exec) {
  return [wprocessor = this->weak_from_this(), exec = move(exec)]() mutable {
    if (auto processor = wprocessor.lock()) {
      processor->execute(move(exec));
    }
  };
}

Execution Processor::willAsyncExecuteOnThis(Execution exec) {
  return [wprocessor = this->weak_from_this(), exec = move(exec)]() mutable {
    if (auto processor = wprocessor.lock()) {
      processor->executeAsync(move(exec));
    }
  };
}

Execution Processor::willBlockingExecuteOnThis(Execution exec) {
  return [wprocessor = this->weak_from_this(), exec = move(exec)]() mutable {
    if (auto processor = wprocessor.lock()) {
      processor->waitableExecute(move(exec)).wait();
    }
  };
}

MsgConnection Processor::connect(const MessageID &msgid,
                                 MessageProcessingCallback processMsgCallback) {
  using namespace std;
  lock_guard lock(d_->msgHandlersMap);
  auto &handlers = (*d_->msgHandlersMap)[msgid];
  if (!handlers) {
    handlers = std::make_shared<Handlers>();
  }
  return new SSConnection(handlers->connect(move(processMsgCallback)));
}

void Processor::disconnect(const MessageID &msgid) {
  std::lock_guard lock(d_->msgHandlersMap);
  d_->msgHandlersMap->erase(msgid);
  d_->cleanupUnconnectedMsgHandlers();
}

size_t Processor::pendingCout() const { return d_->pendingExecutions.size(); }

namespace this_processor {

static bool testAndSetThreadLocalInstance(Processor *inst) {
  if (!instance_) {
    instance_ = inst;
    return true;
  }
  return false;
}

static void clearTLInstanceIfSet(bool set) {
  if (set) {
    instance_ = nullptr;
  }
}

ProcessorInstance instance() {
  if (instance_) {
    return instance_->shared_from_this();
  } else {
    return {};
  }
}

std::weak_ptr<Processor> ref() {
  if (instance_) {
    return instance_->weak_from_this();
  } else {
    return {};
  }
}

bool stop() {
  if (auto comp = instance()) {
    comp->stop();
    return true;
  }
  return false;
}

bool stopped() {
  if (auto comp = instance()) {
    return comp->stopped();
  }
  return false;
}

bool post(Message msg) {
  auto comp = instance();
  return comp ? comp->post(move(msg)) : false;
}

Processor::CompleteSignal waitablePost(Message msg) {
  auto comp = instance();
  return comp ? comp->waitablePost(move(msg)) : CompleteSignal{};
}

bool executeAsync(Execution exec) {
  auto comp = instance();
  return comp ? comp->executeAsync(move(exec)) : false;
}

bool execute(Execution exec) {
  auto comp = instance();
  return comp ? comp->execute(move(exec)) : false;
}

CompleteSignal waitableExecute(Execution exec) {
  auto comp = instance();
  return comp ? comp->waitableExecute(move(exec)) : CompleteSignal{};
}

Processor::Executor getAsyncExecutor() {
  if (auto comp = instance()) {
    return comp->getAsyncExecutor();
  }
  return {};
}

Processor::Executor getExecutor() {
  if (auto comp = instance()) {
    return comp->getExecutor();
  }
  return {};
}

Processor::Executor getWaitableExecutor() {
  if (auto comp = instance()) {
    return comp->getBlockingExecutor();
  }
  return {};
}

Execution willExecuteOnThis(Execution exec) {
  if (auto comp = instance()) {
    return comp->willExecuteOnThis(move(exec));
  }
  return {};
}

Execution willAsyncExecuteOnThis(Execution exec) {
  if (auto comp = instance()) {
    return comp->willAsyncExecuteOnThis(move(exec));
  }
  return {};
}

Execution willBlockingExecuteOnThis(Execution exec) {
  if (auto comp = instance()) {
    return comp->willBlockingExecuteOnThis(move(exec));
  }
  return {};
}

const ProcessorID &id() {
  if (auto comp = instance()) {
    return comp->id();
  }
  return emptyProcessorID();
}

void disconnect(const MessageID &regid) {
  if (auto comp = instance()) {
    comp->disconnect(regid);
  }
}

}  // namespace this_processor

SSConnection *asSSC(void *p) { return reinterpret_cast<SSConnection *>(p); }

MsgConnection::MsgConnection(void *con) { _ = con; }

MsgConnection::MsgConnection(MsgConnection &&con) noexcept {
  _ = con._;
  con._ = nullptr;
}

MsgConnection &MsgConnection::operator=(MsgConnection &&con) noexcept {
  _ = con._;
  con._ = nullptr;
  return *this;
}

MsgConnection::~MsgConnection() { delete asSSC(_); }

bool MsgConnection::connected() const {
  return asSSC(_) ? asSSC(_)->connected() : false;
}

void MsgConnection::disconnect() {
  if (asSSC(_)) {
    asSSC(_)->disconnect();
  }
}

void MsgConnection::reconnect() {
  if (asSSC(_)) {
    asSSC(_)->reconnect();
  }
}

ProcessorInstance makeProcessor(ProcessorID id) {
  return Processor::create(move(id));
}

}  // namespace messaging
}  // namespace maf
