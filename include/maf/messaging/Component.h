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
  MAF_EXPORT static ComponentInstance findComponent(const ComponentID &id);

  MAF_EXPORT const ComponentID &id() const noexcept;

  MAF_EXPORT void run(ThreadFunction threadInit = {},
                      ThreadFunction threadDeinit = {});

  MAF_EXPORT void stop();

  MAF_EXPORT bool post(Message msg);
  MAF_EXPORT bool postAndWait(Message msg);
  MAF_EXPORT bool execute(Execution exec);
  MAF_EXPORT bool executeAndWait(Execution exec);

  MAF_EXPORT Executor getExecutor();

  MAF_EXPORT HandlerRegID registerMessageHandler(MessageID msgid,
                                                 MessageHandler onMessageFunc);

  MAF_EXPORT void unregisterHandler(const HandlerRegID &regid);
  MAF_EXPORT void unregisterAllHandlers(MessageID msgid);

  template <class Msg>
  HandlerRegID onMessage(SpecificMessageHandler<Msg> f) {
    auto &msgID = typeid(Msg);
    auto translatorCallback = [msgName = msgID.name(), callback = std::move(f),
                               this](Message genericMsg) {
      try {
        callback(std::any_cast<Msg>(std::move(genericMsg)));
      } catch (const std::bad_any_cast &) {
        MAF_LOGGER_ERROR("Failed to CAST msg to type of ", msgName);
      } catch (const std::exception &e) {
        MAF_LOGGER_ERROR("Invoking the handler function of message ", msgName,
                         " with exception: ", e.what());
      }
    };

    return registerMessageHandler(msgID, std::move(translatorCallback));
  }

  template <
      class Msg, typename... Args,
      std::enable_if_t<std::is_constructible_v<Msg, Args...>, bool> = true>
  bool post(Args &&... args) {
    return post(Msg{std::forward<Args>(args)...});
  }

  template <
      class Msg, typename... Args,
      std::enable_if_t<std::is_constructible_v<Msg, Args...>, bool> = true>
  bool postAndWait(Args &&... args) {
    return postAndWait(Msg{std::forward<Args>(args)...});
  }

  template <class Msg>
  void unregisterAllHandlers() {
    unregisterAllHandlers(typeid(Msg));
  }

  ~Component();

 private:
  std::unique_ptr<struct ComponentDataPrv> d_;
};

namespace this_component {
MAF_EXPORT std::shared_ptr<Component> instance();
MAF_EXPORT const ComponentID &id();
MAF_EXPORT std::weak_ptr<Component> ref();
MAF_EXPORT bool stop();
MAF_EXPORT bool post(Message msg);
MAF_EXPORT Component::Executor getExecutor();
MAF_EXPORT void unregisterHandler(const HandlerRegID &regid);
MAF_EXPORT void unregisterAllHandlers(const MessageID &regid);

template <class Msg, typename... Args,
          std::enable_if_t<std::is_constructible_v<Msg, Args...>, bool> = true>
static bool post(Args &&... args) {
  return post(Msg{std::forward<Args>(args)...});
}

template <class Msg>
HandlerRegID onMessage(SpecificMessageHandler<Msg> f) {
  return instance()->onMessage<Msg>(std::move(f));
}

template <class Msg>
void unregisterAllHandlers() {
  unregisterAllHandlers(typeid(Msg));
}

};  // namespace this_component

}  // namespace messaging
}  // namespace maf
