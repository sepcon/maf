#pragma once

#include "Component.h"

namespace maf {
namespace messaging {

class ExtensibleComponent {
protected:
  ~ExtensibleComponent() = default;

public:
  using StoppedSignal = Component::StoppedSignal;

  ExtensibleComponent(ComponentID id = {}) {
    _comp = Component::create(std::move(id));
  }

  const std::string &id() const;
  void run();
  StoppedSignal runAsync();
  void stop();
  void post(ComponentMessage &&msg);

  void registerMessageHandler(ComponentMessageID msgid,
                              std::weak_ptr<ComponentMessageHandler> handler);

  void registerMessageHandler(ComponentMessageID msgid,
                              GenericMsgHandlerFunction onMessageFunc);

  template <
      class Msg, typename... Args,
      std::enable_if_t<std::is_constructible_v<Msg, Args...>, bool> = true>
  bool post(Args &&... args) {
    return _comp->post<Msg>(std::forward<Args>(args)...);
  }

  template <class Msg>
  ExtensibleComponent &onMessage(ComponentMessageHandlerFunction<Msg> f) {
    _comp->onMessage<Msg>(f);
    return *this;
  }

  template <class Msg> bool ignoreMessage() {
    return _comp->ignoreMessage<Msg>();
  }

  ComponentInstance component() const { return _comp; }

protected:
  virtual void threadInit() {}
  virtual void threadDeinit() {}

  ComponentInstance _comp;
};

inline const std::string &ExtensibleComponent::id() const {
  return _comp->id();
}

inline void ExtensibleComponent::run() {
  return _comp->run([this] { threadInit(); }, [this] { threadDeinit(); });
}

inline ExtensibleComponent::StoppedSignal ExtensibleComponent::runAsync() {
  return _comp->runAsync([this] { threadInit(); }, [this] { threadDeinit(); });
}

inline void ExtensibleComponent::stop() { _comp->stop(); }

inline void ExtensibleComponent::post(ComponentMessage &&msg) {
  _comp->post(std::move(msg));
}

inline void ExtensibleComponent::registerMessageHandler(
    ComponentMessageID msgid, std::weak_ptr<ComponentMessageHandler> handler) {
  _comp->registerMessageHandler(std::move(msgid), handler);
}

inline void ExtensibleComponent::registerMessageHandler(
    ComponentMessageID msgid, GenericMsgHandlerFunction onMessageFunc) {
  _comp->registerMessageHandler(msgid, std::move(onMessageFunc));
}

} // namespace messaging
} // namespace maf
