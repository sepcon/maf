#pragma once

#include "Component.h"

namespace maf {
namespace messaging {

class ExtensibleComponent {
protected:
  ~ExtensibleComponent() = default;

public:
  ExtensibleComponent(const std::string &name = "") {
    _comp = Component::create();
    _comp->setName(name);
  }

  const std::string &name() const;
  void setName(std::string name);
  void run();
  std::future<void> runAsync();
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

  ComponentPtr component() const { return _comp; }

protected:
  virtual void onEntry() {}
  virtual void onExit() {}

  ComponentPtr _comp;
};

inline const std::string &ExtensibleComponent::name() const {
  return _comp->name();
}

inline void ExtensibleComponent::setName(std::string name) {
  _comp->setName(std::move(name));
}

inline void ExtensibleComponent::run() {
  return _comp->run(std::bind(&ExtensibleComponent::onEntry, this),
                    std::bind(&ExtensibleComponent::onExit, this));
}

inline std::future<void> ExtensibleComponent::runAsync() {
  return _comp->runAsync(std::bind(&ExtensibleComponent::onEntry, this),
                         std::bind(&ExtensibleComponent::onExit, this));
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
