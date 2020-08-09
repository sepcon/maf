#pragma once

#include "Component.h"

namespace maf {
namespace messaging {

class ExtensibleComponent {
 protected:
  ~ExtensibleComponent() = default;

 public:
  ExtensibleComponent(ComponentID id = {}) {
    _comp = Component::create(std::move(id));
  }

  const std::string &id() const;
  void run();
  void stop();
  void post(Message &&msg);

  void registerMessageHandler(MessageID msgid, MessageHandler onMessageFunc);

  template <
      class Msg, typename... Args,
      std::enable_if_t<std::is_constructible_v<Msg, Args...>, bool> = true>
  bool post(Args &&... args) {
    return _comp->post<Msg>(std::forward<Args>(args)...);
  }

  template <class Msg>
  decltype(auto) onMessage(SpecificMessageHandler<Msg> f) {
    return _comp->onMessage<Msg>(f);
  }

  ComponentInstance instance() const { return _comp; }

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

inline void ExtensibleComponent::stop() { _comp->stop(); }

inline void ExtensibleComponent::post(Message &&msg) {
  _comp->post(std::move(msg));
}

inline void ExtensibleComponent::registerMessageHandler(
    MessageID msgid, MessageHandler onMessageFunc) {
  _comp->registerMessageHandler(msgid, std::move(onMessageFunc));
}

}  // namespace messaging
}  // namespace maf
