#include <maf/logging/Logger.h>
#include <maf/messaging/Component.h>
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

namespace this_component {
static bool testAndSetThreadLocalInstance(Component *inst);
static void clearTLInstanceIfSet(bool set);
static thread_local Component *instance_ = nullptr;
}  // namespace this_component

using ExecutionUPtr = std::unique_ptr<Execution>;
class Handlers;
using HandlersPtr = std::shared_ptr<Handlers>;
using PendingExecutions = threading::Queue<ExecutionUPtr>;
using MsgHandlersMap = threading::Lockable<std::map<MessageID, HandlersPtr>>;
using util::CallOnExit;

static inline constexpr auto anonymous_prefix = "[anonymous]."sv;

class CallbackExecutor : public util::ExecutorIF {
  ComponentRef compref;

 public:
  CallbackExecutor(ComponentRef &&cr) : compref{std::move(cr)} {}
  bool execute(CallbackType callback) noexcept override {
    if (auto comp = compref.lock()) {
      comp->execute(std::move(callback));
      return true;
    }
    return false;
  }
};

class BlockingCallbackExecutor : public util::ExecutorIF {
  ComponentRef compref;

 public:
  BlockingCallbackExecutor(ComponentRef &&cr) : compref{std::move(cr)} {}
  bool execute(CallbackType callback) noexcept override {
    if (auto comp = compref.lock()) {
      try {
        comp->execute(Blocked, std::move(callback)).wait();
        return true;
      } catch (const std::future_error &) {
        return false;
      }
    }
    return false;
  }
};

class Handlers {
 public:
  using Handler = MessageProcessingCallback;
  using HandlerList = std::forward_list<Handler>;
  using HandlerID = HandlerList::pointer;
  using HandlerIt = HandlerList::iterator;

  HandlerID add(Handler handler) {
    std::lock_guard lock(handlers_);
    handlers_->push_front(std::move(handler));
    return asID(handlers_->begin());
  }

  void remove(HandlerID handlerID) {
    std::lock_guard lock(handlers_);
    auto beg = std::begin(*handlers_);
    auto end = std::end(*handlers_);
    auto prevRemoved = handlers_->before_begin();
    while (beg != end) {
      if (handlerID == asID(beg)) {
        handlers_->erase_after(prevRemoved);
        break;
      } else {
        prevRemoved = beg++;
      }
    }
  }

  void handle(const Message &msg) const {
    std::lock_guard lock(handlers_);
    handle(std::begin(*handlers_), std::end(*handlers_), msg);
  }

  bool empty() const { return handlers_.atomic()->empty(); }
  template <class Iterator>
  static void handle(Iterator beg, Iterator end, const Message &msg) {
    if (beg != end) {
      auto &handler = *beg;
      handle(++beg, end, msg);
      handler(msg);
    }
  }

  static HandlerID asID(HandlerIt it) { return it.operator->(); }

 private:
  threading::Lockable<HandlerList, std::recursive_mutex> handlers_;
};

struct ComponentDataPrv {
  ComponentDataPrv(ComponentID id) : id{std::move(id)} {}
  ComponentID id;
  PendingExecutions pendingExecutions;
  MsgHandlersMap msgHandlersMap;

  HandlersPtr findHandlers(const MessageID &msgID) {
    std::lock_guard lock(msgHandlersMap);
    auto itHandler = msgHandlersMap->find(msgID);
    if (itHandler != msgHandlersMap->end()) {
      return itHandler->second;
    }
    return {};
  }

  void closeAndClearExecutionsQueue() {
    pendingExecutions.close();
    pendingExecutions.clear();
  }
};

static decltype(auto) invoke(const ExecutionUPtr &exc) { return (*exc)(); }

static ConnectionID makeRegID(Handlers::HandlerID hid, MessageID mid) {
  return ConnectionID{hid, mid};
}

static const ComponentID &emptyComponentID() {
  static ComponentID emptyID;
  return emptyID;
}

static ComponentID generateAnonymousID() {
  static std::atomic_int counter = 0;
  return anonymous_prefix.data() + std::to_string(++counter);
}

static bool isAnonymous(const ComponentID &id) {
  return id.length() > anonymous_prefix.length() &&
         std::string_view(id.c_str(), anonymous_prefix.length()) ==
             anonymous_prefix;
}

static void joinRoutingIfNotAnonymous(ComponentInstance comp) {
  if (!isAnonymous(comp->id())) {
    Router::instance().addComponent(comp);
  }
}

static void leaveRoutingIfNotAnonymous(ComponentInstance comp) {
  if (!isAnonymous(comp->id())) {
    Router::instance().removeComponent(comp);
  }
}

Component::Component(ComponentID id)
    : d_{new ComponentDataPrv{std::move(id)}} {}

Component::~Component() { d_->closeAndClearExecutionsQueue(); }

ComponentInstance Component::create(ComponentID id) {
  auto willJoinRouting = !id.empty();
  if (willJoinRouting) {
    assert(!isAnonymous(id));
    // Make sure the Router::instance is created before all Component instances
    // that depend on it
    Router::instance();
  } else {
    id = generateAnonymousID();
  }

  auto comp = ComponentInstance{new Component{std::move(id)}};

  if (willJoinRouting) {
    if (Router::instance().findComponent(comp->id())) {
      comp.reset();
    } else {
      Router::instance().addComponent(comp);
    }
  }
  return comp;
}

ComponentInstance Component::findComponent(const ComponentID &id) {
  return Router::instance().findComponent(id);
}

const ComponentID &Component::id() const noexcept { return d_->id; }

void Component::run(ThreadFunction threadInit, ThreadFunction threadDeinit) {
  auto justSet = this_component::testAndSetThreadLocalInstance(this);
  if (threadInit) {
    threadInit();
  }

  CallOnExit deinit = [justSet, threadDeinit = std::move(threadDeinit)] {
    if (threadDeinit) {
      threadDeinit();
    }
    this_component::clearTLInstanceIfSet(justSet);
  };

  ExecutionUPtr exc;
  while (d_->pendingExecutions.wait(exc)) {
    invoke(exc);
  }
}

void Component::runFor(ExecutionTimeout duration) {
  using namespace std::chrono;
  runUntil(system_clock::now() + duration);
}

void Component::runUntil(ExecutionDeadline deadline) {
  ExecutionUPtr exc;
  auto justSet = this_component::testAndSetThreadLocalInstance(this);
  CallOnExit deinit = [justSet] {
    this_component::clearTLInstanceIfSet(justSet);
  };

  while (d_->pendingExecutions.waitUntil(exc, deadline)) {
    invoke(exc);
  }
}

void Component::stop() {
  if (!stopped()) {
    d_->closeAndClearExecutionsQueue();
    leaveRoutingIfNotAnonymous(shared_from_this());
  }
}

bool Component::stopped() const { return d_->pendingExecutions.isClosed(); }

bool Component::post(Message msg) {
  using namespace std;
  if (!stopped()) {
    auto &msgType = msg.type();
    if (auto handlers = d_->findHandlers(msgType)) {
      return execute([handlers = move(handlers), msg = move(msg)] {
        handlers->handle(msg);
      });
    } else {
      MAF_LOGGER_WARN("There's no handler for message ", msgType.name());
    }
  }
  return false;
}

Component::CompleteSignal Component::send(Message msg) {
  using namespace std;
  CompleteSignal doneSignal;
  if (!stopped()) {
    auto &msgType = msg.type();
    if (auto handlers = d_->findHandlers(msgType)) {
      auto msgHandlingTask = make_shared<packaged_task<void()>>(
          [handlers = move(handlers), msg = move(msg)] {
            handlers->handle(msg);
          });

      doneSignal = CompleteSignal{msgHandlingTask->get_future()};
      if (this_component::id() != id()) {
        execute([task{move(msgHandlingTask)}] { (*task)(); });
      } else {
        (*msgHandlingTask)();
      }
    } else {
      MAF_LOGGER_WARN("There's no handler for message ", msgType.name());
    }
  }
  return doneSignal;
}

bool Component::connected(const MessageID &mid) const {
  return d_->msgHandlersMap.atomic()->count(mid) > 0;
}

bool Component::execute(Execution exec) {
  using namespace std;
  if (!stopped()) {
    try {
      d_->pendingExecutions.push(make_unique<Execution>(move(exec)));
      return true;
    } catch (const bad_alloc &ba) {
      MAF_LOGGER_ERROR("Queue overflow: ", ba.what());
    }
  }
  return false;
}

Component::CompleteSignal Component::execute(BlockingMode, Execution exec) {
  using namespace std;
  CompleteSignal doneSignal;
  if (!stopped()) {
    auto task = make_shared<packaged_task<void()>>(move(exec));
    doneSignal = CompleteSignal{task->get_future()};
    if (this_component::id() != id()) {
      execute([task{move(task)}] { (*task)(); });
    } else {
      (*task)();
    }
  }
  return doneSignal;
}

Component::Executor Component::getExecutor() {
  return std::make_shared<CallbackExecutor>(weak_from_this());
}

Component::Executor Component::getBlockingExecutor() {
  return std::make_shared<BlockingCallbackExecutor>(weak_from_this());
}

ConnectionID Component::connect(const MessageID &msgid,
                                MessageProcessingCallback processMessage) {
  using namespace std;
  lock_guard lock(d_->msgHandlersMap);
  auto &handlers = (*d_->msgHandlersMap)[msgid];
  if (!handlers) {
    handlers = make_shared<Handlers>();
  }

  return makeRegID(handlers->add(move(processMessage)), msgid);
}

void Component::disconnect(const ConnectionID &regid) {
  std::lock_guard lock(d_->msgHandlersMap);
  if (auto itHandlers = d_->msgHandlersMap->find(regid.mid_);
      itHandlers != d_->msgHandlersMap->end()) {
    auto &handlers = itHandlers->second;
    handlers->remove(reinterpret_cast<Handlers::HandlerID>(regid.hid_));
    if (handlers->empty()) {
      d_->msgHandlersMap->erase(itHandlers);
    }
  }
}

void Component::disconnect(const MessageID &msgid) {
  d_->msgHandlersMap.atomic()->erase(msgid);
}

size_t Component::pendingCout() const { return d_->pendingExecutions.size(); }

namespace this_component {

static bool testAndSetThreadLocalInstance(Component *inst) {
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

ComponentInstance instance() {
  if (instance_) {
    return instance_->shared_from_this();
  } else {
    return {};
  }
}

std::weak_ptr<Component> ref() {
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
  if (auto comp = instance()) {
    return comp->post(std::move(msg));
  }
  return false;
}

Component::Executor getExecutor() {
  if (auto comp = instance()) {
    return comp->getExecutor();
  }
  return {};
}

Component::Executor getBlockingExecutor() {
  if (auto comp = instance()) {
    return comp->getBlockingExecutor();
  }
  return {};
}

const ComponentID &id() {
  if (auto comp = instance()) {
    return comp->id();
  }
  return emptyComponentID();
}

void disconnect(const ConnectionID &regid) {
  if (auto comp = instance()) {
    comp->disconnect(regid);
  }
}
void disconnect(const MessageID &regid) {
  if (auto comp = instance()) {
    comp->disconnect(regid);
  }
}
}  // namespace this_component
}  // namespace messaging
}  // namespace maf
