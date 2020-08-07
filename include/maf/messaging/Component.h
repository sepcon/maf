#pragma once

#include <maf/export/MafExport_global.h>
#include <maf/logging/Logger.h>
#include <maf/patterns/Patterns.h>

#include "ComponentDef.h"
#include "ExecutorIF.h"

namespace maf {
namespace messaging {

class Component final : pattern::Unasignable,
                        public std::enable_shared_from_this<Component> {
  MAF_EXPORT Component(ComponentID id);

 public:
  using ThreadFunction = std::function<void()>;
  using Executor = std::shared_ptr<ExecutorIF>;

  MAF_EXPORT static ComponentInstance create(ComponentID id = {});
  MAF_EXPORT static ComponentInstance findComponent(
      const ComponentID &id) noexcept;

  MAF_EXPORT const ComponentID &id() const noexcept;

  MAF_EXPORT void run(ThreadFunction threadInit = {},
                      ThreadFunction threadDeinit = {});

  MAF_EXPORT void stop() noexcept;

  MAF_EXPORT bool post(ComponentMessage msg) noexcept;
  MAF_EXPORT bool execute(Execution exec) noexcept;
  MAF_EXPORT bool executeAndWait(Execution exec);

  MAF_EXPORT Executor getExecutor();

  MAF_EXPORT void registerMessageHandler(
      ComponentMessageID msgid, GenericMsgHandlerFunction onMessageFunc);

  MAF_EXPORT bool unregisterHandler(ComponentMessageID msgid) noexcept;

  template <class Msg>
  Component *onMessage(ComponentMessageHandlerFunction<Msg> f) {
    auto &msgID = typeid(Msg);
    auto translatorCallback = [msgName = msgID.name(), callback = std::move(f),
                               this](ComponentMessage genericMsg) {
      try {
        callback(std::any_cast<Msg>(std::move(genericMsg)));
      } catch (const std::bad_any_cast &) {
        MAF_LOGGER_ERROR("Failed to CAST msg to type of ", msgName);
      } catch (const std::exception &e) {
        MAF_LOGGER_ERROR("Invoking the handler function of message ", msgName,
                         " with exception: ", e.what());
      }
    };

    registerMessageHandler(msgID, std::move(translatorCallback));
    return this;
  }

  template <
      class Msg, typename... Args,
      std::enable_if_t<std::is_constructible_v<Msg, Args...>, bool> = true>
  bool post(Args &&... args) noexcept {
    return post(Msg{std::forward<Args>(args)...});
  }

  template <class Msg>
  bool ignoreMessage() noexcept {
    return this->unregisterHandler(typeid(Msg));
  }

 private:
  MAF_EXPORT ~Component();
  static void deleteFunction(Component *comp);

  std::unique_ptr<struct ComponentDataPrv> d_;
};

namespace this_component {
MAF_EXPORT std::shared_ptr<Component> instance() noexcept;
MAF_EXPORT std::weak_ptr<Component> ref() noexcept;
MAF_EXPORT bool stop() noexcept;
MAF_EXPORT bool post(ComponentMessage &&msg) noexcept;
MAF_EXPORT Component::Executor getExecutor();

template <class Msg, typename... Args,
          std::enable_if_t<std::is_constructible_v<Msg, Args...>, bool> = true>
static bool post(Args &&... args) {
  return post(Msg{std::forward<Args>(args)...});
}

template <class Msg>
static bool ignoreMessage() {
  if (auto comp = instance()) {
    return comp->ignoreMessage<Msg>();
  }
  return false;
}
};  // namespace this_component

}  // namespace messaging
}  // namespace maf
