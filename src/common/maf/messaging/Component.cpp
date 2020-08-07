#include <maf/logging/Logger.h>
#include <maf/messaging/Component.h>
#include <maf/threading/Lockable.h>
#include <maf/threading/Queue.h>

#include <cassert>
#include <future>
#include <map>

#include "Router.h"

namespace maf {
namespace messaging {

using PendingExecutions = threading::Queue<Execution>;
using HandlerMap = threading::Lockable<
    std::map<ComponentMessageID, GenericMsgHandlerFunction>>;

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

struct ComponentDataPrv {
  ComponentDataPrv(ComponentID id) : id{std::move(id)} {}
  ComponentID id;
  PendingExecutions pendingExecutions;
  HandlerMap handlers;

  GenericMsgHandlerFunction findHandler(const ComponentMessageID &msgID) {
    std::lock_guard lock(handlers);
    auto itHandler = handlers->find(msgID);
    if (itHandler != handlers->end()) {
      return itHandler->second;
    }
    return {};
  }
};

static thread_local Component *tlInstance_ = nullptr;

Component::Component(ComponentID id)
    : d_{new ComponentDataPrv{std::move(id)}} {}

Component::~Component() {}

ComponentInstance Component::create(ComponentID name) {
  if (!name.empty()) {
    // Make sure the Router::instance is created before all Component instances
    // that depend on it
    Router::instance();
  }

  auto comp = ComponentInstance{new Component{std::move(name)},
                                &Component::deleteFunction};
  if (!comp->id().empty()) {
    if (!Router::instance().addReceiver(comp)) {
      comp.reset();
    }
  }
  return comp;
}

ComponentInstance Component::findComponent(const ComponentID &id) noexcept {
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

void Component::stop() noexcept {
  d_->pendingExecutions.close();
  d_->pendingExecutions.clear();
  if (!id().empty()) {
    Router::instance().removeReceiver(shared_from_this());
  }
}

bool Component::post(ComponentMessage msg) noexcept {
  if (!d_->pendingExecutions.isClosed()) {
    auto &msgType = msg.type();
    assert(msgType != typeid(std::nullptr_t));

    if (auto handlerFunc = d_->findHandler(msgType)) {
      return execute(std::bind(std::move(handlerFunc), std::move(msg)));
    } else {
      MAF_LOGGER_WARN("There's no handler for message ", msgType.name());
    }
  }
  return false;
}

bool Component::execute(Execution exec) noexcept {
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

void Component::registerMessageHandler(
    ComponentMessageID msgid, GenericMsgHandlerFunction onMessageFunc) {
  if (onMessageFunc) {
    d_->handlers.atomic()->emplace(std::move(msgid), std::move(onMessageFunc));
  }
}

bool Component::unregisterHandler(ComponentMessageID msgid) noexcept {
  std::lock_guard lock(d_->handlers);
  return d_->handlers->erase(msgid) > 0;
}

void Component::deleteFunction(Component *comp) { delete comp; }

ComponentInstance this_component::instance() noexcept {
  if (tlInstance_) {
    return tlInstance_->shared_from_this();
  } else {
    return {};
  }
}

std::weak_ptr<Component> this_component::ref() noexcept {
  if (tlInstance_) {
    return tlInstance_->weak_from_this();
  } else {
    return {};
  }
}

bool this_component::stop() noexcept {
  if (auto comp = instance()) {
    comp->stop();
    return true;
  }
  return false;
}

bool this_component::post(ComponentMessage &&msg) noexcept {
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

}  // namespace messaging
}  // namespace maf
