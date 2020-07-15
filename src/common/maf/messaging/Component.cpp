#include <maf/logging/Logger.h>
#include <maf/messaging/Component.h>
#include <maf/threading/Lockable.h>
#include <maf/threading/Queue.h>

#include <cassert>
#include <map>

#include "MessageRouter.h"

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

static Execution makeExecution(GenericMsgHandlerFunction &&handler,
                               ComponentMessage &&msg) {
  auto executeMsgHandler = [handler = std::move(handler),
                            msg = std::move(msg)] {
    auto &msgType = msg.type();
    try {
      handler(std::move(msg));
    } catch (const std::exception &e) {
      MAF_LOGGER_ERROR(
          "Exception occurred while executing "
          "messageHandler function of msg `",
          msgType.name(), "`: ", e.what());
    } catch (...) {
      MAF_LOGGER_ERROR(
          "Unknown exception occurred while executing"
          " messageHandler function: ");
    }
  };

  return executeMsgHandler;
}

Component::Component(ComponentID id)
    : d_{new ComponentDataPrv{std::move(id)}} {}

Component::~Component() {}

ComponentInstance Component::create(ComponentID name) {
  auto comp = ComponentInstance{new Component{std::move(name)},
                                &Component::deleteFunction};
  if (!comp->id().empty()) {
    if (!MessageRouter::instance().addReceiver(comp)) {
      comp.reset();
    }
  }
  return comp;
}

ComponentInstance Component::findComponent(const ComponentID &id) {
  return routing::findReceiver(id);
}

const ComponentID &Component::id() const { return d_->id; }

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

Component::StoppedSignal Component::runAsync(ThreadFunction threadInit,
                                             ThreadFunction threadDeinit) {
  return std::async(std::launch::async,
                    std::bind(&Component::run, this, std::move(threadInit),
                              std::move(threadDeinit)));
}

void Component::stop() {
  d_->pendingExecutions.close();
  MessageRouter::instance().removeReceiver(shared_from_this());
}

bool Component::post(ComponentMessage &&msg) {
  auto &msgType = msg.type();
  assert(msgType != typeid(std::nullptr_t));

  if (auto handlerFunc = d_->findHandler(msgType)) {
    execute(makeExecution(std::move(handlerFunc), std::move(msg)));
  } else {
    MAF_LOGGER_WARN("There's no handler for message ", msgType.name());
  }

  return false;
}

bool Component::post(const ComponentMessage &msg) {
  auto msgCopied = msg;
  return post(std::move(msgCopied));
}

bool Component::execute(Execution &&exec) {
  try {
    d_->pendingExecutions.push(std::move(exec));
    return true;
  } catch (const std::bad_alloc &ba) {
    MAF_LOGGER_ERROR("Queue overflow: ", ba.what());
  }
  return false;
}

Component::Executor Component::getExecutor() {
  return std::make_shared<CallbackExecutor>(weak_from_this());
}

void Component::registerMessageHandler(
    ComponentMessageID msgid, std::weak_ptr<ComponentMessageHandler> handler) {
  auto handlerFunction = [handler = std::move(handler)](ComponentMessage msg) {
    if (auto handlerPtr = handler.lock()) {
      handlerPtr->onMessage(std::move(msg));
    }
  };

  registerMessageHandler(msgid, std::move(handlerFunction));
}

void Component::registerMessageHandler(
    ComponentMessageID msgid, GenericMsgHandlerFunction onMessageFunc) {
  if (onMessageFunc) {
    d_->handlers.atomic()->emplace(std::move(msgid), std::move(onMessageFunc));
  }
}

bool Component::unregisterHandler(ComponentMessageID msgid) {
  std::lock_guard lock(d_->handlers);
  return d_->handlers->erase(msgid) > 0;
}

void Component::deleteFunction(Component *comp) { delete comp; }

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

bool this_component::post(ComponentMessage &&msg) {
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
