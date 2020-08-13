#include <maf/logging/Logger.h>
#include <maf/messaging/Component.h>
#include <maf/threading/Lockable.h>
#include <maf/threading/Queue.h>

#include <cassert>
#include <forward_list>
#include <future>
#include <map>

#include "Router.h"

namespace maf {
namespace messaging {

class Handlers;
using HandlersPtr = std::shared_ptr<Handlers>;
using PendingExecutions = threading::Queue<Execution>;
using MsgHandlersMap = threading::Lockable<std::map<MessageID, HandlersPtr>>;

static const ComponentID anonymous_prefix = "[anonymous].";

class CallbackExecutor : public ExecutorIF {
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

class Handlers {
 public:
  using Handler = MessageHandler;
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
};

static thread_local Component *tlInstance_ = nullptr;

static HandlerRegID makeRegID(Handlers::HandlerID hid, MessageID mid) {
  return HandlerRegID{hid, mid};
}

static const ComponentID &emptyComponentID() {
  static ComponentID emptyID;
  return emptyID;
}

static ComponentID generateAnonymousID() {
  static std::atomic_int counter = 0;
  return anonymous_prefix + std::to_string(++counter);
}

static bool isAnonymous(const ComponentID &id) {
  return id.length() > anonymous_prefix.length() &&
         strncmp(id.c_str(), anonymous_prefix.c_str(),
                 anonymous_prefix.length()) == 0;
}

Component::Component(ComponentID id)
    : d_{new ComponentDataPrv{std::move(id)}} {}

Component::~Component() {}

ComponentInstance Component::create(ComponentID id) {
  auto joinRouting = !id.empty();
  if (joinRouting) {
    // Make sure the Router::instance is created before all Component instances
    // that depend on it
    Router::instance();
  } else {
    id = generateAnonymousID();
  }

  auto comp = ComponentInstance{new Component{std::move(id)}};

  if (joinRouting) {
    if (!Router::instance().addReceiver(comp)) {
      comp.reset();
    }
  }
  return comp;
}

ComponentInstance Component::findComponent(const ComponentID &id) {
  return routing::findReceiver(id);
}

const ComponentID &Component::id() const noexcept { return d_->id; }

void Component::run(ThreadFunction threadInit, ThreadFunction threadDeinit) {
  tlInstance_ = this;
  if (threadInit) {
    threadInit();
  }

  Execution exc;
  while (d_->pendingExecutions.wait(exc)) {
    exc();
  }
  if (threadDeinit) {
    threadDeinit();
  }
}

void Component::stop() {
  if (!d_->pendingExecutions.isClosed()) {
    d_->pendingExecutions.close();
    d_->pendingExecutions.clear();
    if (!isAnonymous(id())) {
      Router::instance().removeReceiver(shared_from_this());
    }
  }
}

bool Component::post(Message msg) {
  using namespace std;
  if (!d_->pendingExecutions.isClosed()) {
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

bool Component::postAndWait(Message msg) {
  using namespace std;
  if (!d_->pendingExecutions.isClosed()) {
    auto &msgType = msg.type();
    if (auto handlers = d_->findHandlers(msgType)) {
      if (id() == this_component::id()) {
        handlers->handle(msg);
        return true;
      }
      auto handlersInvoker = make_shared<packaged_task<void()>>(
          [handlers = move(handlers), msg = move(msg)] {
            handlers->handle(msg);
          });

      auto handledEvent = handlersInvoker->get_future();
      if (execute([msgHandler = move(handlersInvoker)] { (*msgHandler)(); })) {
        try {
          handledEvent.get();
          return true;
        } catch (const std::future_error &fe) {
          MAF_LOGGER_WARN(
              "Seems that the component that was asked for executing function "
              "has been stopped, exception: ",
              fe.what());
        }
      }
    } else {
      MAF_LOGGER_WARN("There's no handler for message ", msgType.name());
    }
  }
  return false;
}

bool Component::execute(Execution exec) {
  if (!d_->pendingExecutions.isClosed()) {
    try {
      d_->pendingExecutions.push(std::move(exec));
      return true;
    } catch (const std::bad_alloc &ba) {
      MAF_LOGGER_ERROR("Queue overflow: ", ba.what());
    }
  }
  return false;
}

bool Component::executeAndWait(Execution exec) {
  using namespace std;
  auto ret = false;

  if (!d_->pendingExecutions.isClosed()) {
    if (id() == this_component::id()) {
      exec();
      return true;
    }
    auto task = make_shared<packaged_task<void()>>(move(exec));
    auto ft = task->get_future();
    if (execute([task = move(task)] { (*task)(); })) {
      try {
        ft.get();
        ret = true;
      } catch (const std::future_error &fe) {
        MAF_LOGGER_WARN(
            "Seems that the component that was asked for executing function "
            "has been stopped, exception: ",
            fe.what());
      }
    }
  }

  return ret;
}

Component::Executor Component::getExecutor() {
  return std::make_shared<CallbackExecutor>(weak_from_this());
}

HandlerRegID Component::registerMessageHandler(MessageID msgid,
                                               MessageHandler handler) {
  using namespace std;
  lock_guard lock(d_->msgHandlersMap);
  auto &handlers = (*d_->msgHandlersMap)[msgid];
  if (!handlers) {
    handlers = make_shared<Handlers>();
  }

  return makeRegID(handlers->add(move(handler)), msgid);
}

void Component::unregisterHandler(const HandlerRegID &regid) {
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

void Component::unregisterAllHandlers(MessageID msgid) {
  d_->msgHandlersMap.atomic()->erase(msgid);
}

ComponentInstance this_component::instance() {
  if (tlInstance_) {
    return tlInstance_->shared_from_this();
  } else {
    return {};
  }
}

std::weak_ptr<Component> this_component::ref() {
  if (tlInstance_) {
    return tlInstance_->weak_from_this();
  } else {
    return {};
  }
}

bool this_component::stop() {
  if (auto comp = instance()) {
    comp->stop();
    return true;
  }
  return false;
}

bool this_component::post(Message msg) {
  if (auto comp = instance()) {
    return comp->post(std::move(msg));
  }
  return false;
}

Component::Executor this_component::getExecutor() {
  if (auto comp = instance()) {
    return comp->getExecutor();
  }
  return {};
}

const ComponentID &this_component::id() {
  if (auto comp = instance()) {
    return comp->id();
  }
  return emptyComponentID();
}

void this_component::unregisterHandler(const HandlerRegID &regid) {
  if (auto comp = instance()) {
    comp->unregisterHandler(regid);
  }
}
void this_component::unregisterAllHandlers(const MessageID &regid) {
  if (auto comp = instance()) {
    comp->unregisterAllHandlers(regid);
  }
}

}  // namespace messaging
}  // namespace maf
