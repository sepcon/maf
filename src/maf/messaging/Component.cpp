#include <cassert>
#include <maf/logging/Logger.h>
#include <maf/messaging/BasicMessages.h>
#include <maf/messaging/Component.h>
#include <maf/threading/Lockable.h>
#include <maf/threading/Queue.h>
#include <map>
#include <memory>

namespace maf {
namespace messaging {

using MessageQueue = threading::Queue<std::any>;
using HandlerMap = threading::Lockable<
    std::map<ComponentMessageID, GenericMsgHandlerFunction>>;

struct ComponentDataPrv {
  ComponentDataPrv(std::string name_) : name{std::move(name_)} {}
  std::string name;
  MessageQueue msgq;
  HandlerMap handlers;
};

static thread_local Component *tlInstance_ = nullptr;

Component::Component(std::string name)
    : d_{new ComponentDataPrv{std::move(name)}} {
  onMessage<CallbackExcMsg>([](CallbackExcMsg msg) { msg.execute(); });
}

Component::~Component() { stop(); }

std::shared_ptr<Component> Component::create(std::string name) {
  return std::shared_ptr<Component>{new Component{std::move(name)},
                                    &Component::deleteFunction};
}

const std::string &Component::name() const { return d_->name; }

void Component::setName(std::string name) { d_->name = std::move(name); }

void Component::run(std::function<void()> onEntry,
                    std::function<void()> onExit) {
  tlInstance_ = this;
  if (onEntry) {
    onEntry();
  }

  ComponentMessage msg;
  while (d_->msgq.wait(msg)) {
    auto &msgType = msg.type();
    GenericMsgHandlerFunction handlerFunc;
    {
      std::lock_guard lock(d_->handlers);
      auto itHandler = d_->handlers->find(msgType);
      if (itHandler != d_->handlers->end()) {
        handlerFunc = itHandler->second;
      }
    }

    if (handlerFunc) {
      try {
        handlerFunc(msg);
      } catch (const std::exception &e) {
        MAF_LOGGER_ERROR("Exception occurred while executing "
                         "messageHandler function of msg `",
                         msgType.name(), "`: ", e.what());
      } catch (...) {
        MAF_LOGGER_ERROR("Unknown exception occurred while executing"
                         " messageHandler function: ");
      }
    } else {
      MAF_LOGGER_WARN("There's no handler for message ", msgType.name());
    }
  }

  if (onExit) {
    onExit();
  }
}

std::future<void> Component::runAsync(std::function<void()> onEntry,
                                      std::function<void()> onExit) {
  return std::async(
      std::launch::async,
      std::bind(&Component::run, this, std::move(onEntry), std::move(onExit)));
}

void Component::stop() { d_->msgq.close(); }

bool Component::post(ComponentMessage &&msg) {
  try {
    assert(msg.type() != typeid(nullptr_t));
    d_->msgq.push(std::move(msg));
    return true;
  } catch (const std::bad_alloc &ba) {
    MAF_LOGGER_ERROR("Queue overflow: ", ba.what());
  } catch (const std::exception &e) {
    MAF_LOGGER_ERROR("Exception occurred when pushing data to queue: ",
                     e.what());
  } catch (...) {
    MAF_LOGGER_ERROR("Unkown exception occurred when pushing data to queue!");
  }
  return false;
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

std::shared_ptr<Component> RunningComponent::shared() {
  if (tlInstance_) {
    return tlInstance_->shared_from_this();
  } else {
    return {};
  }
}

std::weak_ptr<Component> RunningComponent::weak() {
  if (tlInstance_) {
    return tlInstance_->weak_from_this();
  } else {
    return {};
  }
}

bool RunningComponent::stop() {
  if (auto comp = shared()) {
    comp->stop();
    return false;
  }
  return false;
}

bool RunningComponent::post(ComponentMessage &&msg) {
  if (auto comp = shared()) {
    return comp->post(std::move(msg));
  }
  return false;
}

} // namespace messaging
} // namespace maf
