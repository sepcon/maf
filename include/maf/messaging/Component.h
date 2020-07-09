#pragma once

#include <maf/export/MafExport_global.h>
#include <maf/logging/Logger.h>
#include <maf/patterns/Patterns.h>

#include <future>

#include "ComponentDef.h"
#include "ExecutorIF.h"

namespace maf {
namespace messaging {

class Component final : pattern::Unasignable,
                        public std::enable_shared_from_this<Component> {
  std::unique_ptr<struct ComponentDataPrv> d_;
  MAF_EXPORT Component(ComponentID id);

  using ThreadFunction = std::function<void()>;

public:
  using StoppedSignal = std::future<void>;
  using Executor = std::shared_ptr<ExecutorIF>;

  MAF_EXPORT static std::shared_ptr<Component> create(ComponentID id = {});
  MAF_EXPORT const ComponentID &id() const;

  MAF_EXPORT void run(ThreadFunction threadInit = {},
                      ThreadFunction threadDeinit = {});

  MAF_EXPORT StoppedSignal runAsync(ThreadFunction threadInit = {},
                                   ThreadFunction threadDeinit = {});

  MAF_EXPORT void stop();

  MAF_EXPORT bool post(ComponentMessage &&msg);
  MAF_EXPORT bool post(const ComponentMessage &msg);
  MAF_EXPORT bool execute(Execution &&exec);
  MAF_EXPORT Executor getExecutor();

  MAF_EXPORT void
  registerMessageHandler(ComponentMessageID msgid,
                         std::weak_ptr<ComponentMessageHandler> handler);

  MAF_EXPORT void
  registerMessageHandler(ComponentMessageID msgid,
                         GenericMsgHandlerFunction onMessageFunc);

  MAF_EXPORT bool unregisterHandler(ComponentMessageID msgid);

  template <class Msg>
  Component *onMessage(ComponentMessageHandlerFunction<Msg> f) {
    auto translatorCallback = [callback = std::move(f),
                               this](ComponentMessage genericMsg) {
      try {
        callback(std::any_cast<Msg>(std::move(genericMsg)));
      } catch (const std::bad_any_cast &) {
        MAF_LOGGER_ERROR("Failed to CAST msg to type of ", typeid(Msg).name());
      }
    };
    registerMessageHandler(typeid(Msg), std::move(translatorCallback));
    return this;
  }

  template <
      class Msg, typename... Args,
      std::enable_if_t<std::is_constructible_v<Msg, Args...>, bool> = true>
  bool post(Args &&... args) {
    return post(Msg{std::forward<Args>(args)...});
  }

  template <class Msg> bool ignoreMessage() {
    return this->unregisterHandler(typeid(Msg));
  }

private:
  MAF_EXPORT ~Component();
  static void deleteFunction(Component *comp);
};

class ComponentMessageHandler {
public:
  virtual void onMessage(ComponentMessage msg) = 0;
  virtual ~ComponentMessageHandler() = default;
};

namespace this_component {
MAF_EXPORT std::shared_ptr<Component> instance();
MAF_EXPORT std::weak_ptr<Component> ref();
MAF_EXPORT bool stop();
MAF_EXPORT bool post(ComponentMessage &&msg);
MAF_EXPORT Component::Executor getExecutor();

template <class Msg, typename... Args,
          std::enable_if_t<std::is_constructible_v<Msg, Args...>, bool> = true>
static bool post(Args &&... args) {
  return post(Msg{std::forward<Args>(args)...});
}

template <class Msg>
static bool onMessage(ComponentMessageHandlerFunction<Msg> f) {
  if (auto comp = instance()) {
    comp->onMessage<Msg>(std::move(f));
    return true;
  }
  return false;
}

template <class Msg> static bool ignoreMessage() {
  if (auto comp = instance()) {
    return comp->ignoreMessage<Msg>();
  }
  return false;
}
}; // namespace this_component

} // namespace messaging
} // namespace maf
