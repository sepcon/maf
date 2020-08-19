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
  MAF_EXPORT void runFor(ExecutionTimeout duration);
  MAF_EXPORT void runUntil(ExecutionDeadline deadline);
  MAF_EXPORT void stop();
  MAF_EXPORT bool stopped() const;
  MAF_EXPORT bool post(Message msg);
  MAF_EXPORT bool hasHandler(MessageID mid) const;
  MAF_EXPORT bool postAndWait(Message msg);
  MAF_EXPORT bool execute(Execution exec);
  MAF_EXPORT bool executeAndWait(Execution exec);
  MAF_EXPORT Executor getExecutor();
  MAF_EXPORT HandlerRegID registerMessageHandler(MessageID msgid,
                                                 MessageHandler onMessageFunc);
  MAF_EXPORT void unregisterHandler(const HandlerRegID &regid);
  MAF_EXPORT void unregisterAllHandlers(MessageID msgid);
  MAF_EXPORT size_t pendingCout() const;

  template <class Msg>
  HandlerRegID onMessage(SpecificMessageHandler<Msg> f) {
    auto translatorCallback = [callback = std::move(f),
                               this](const Message &genericMsg) {
      try {
        callback(std::any_cast<const Msg &>(genericMsg));
      } catch (const std::bad_any_cast &) {
        MAF_LOGGER_ERROR("Failed to CAST msg to type of ", msgid<Msg>().name());
      } catch (const std::exception &e) {
        MAF_LOGGER_FATAL("EXCEPTION when handling message ",
                         msgid<Msg>().name(), ": ", e.what());
        throw;
      }
    };

    return registerMessageHandler(msgid<Msg>(), std::move(translatorCallback));
  }

  template <class Msg>
  HandlerRegID onMessage(DontCareMsgContentHandler f) {
    return registerMessageHandler(msgid<Msg>(),
                                  [f{std::move(f)}](const auto &) { f(); });
  }

  template <class Msg, typename... Args>
  bool post(Args &&... args) {
    return post(makeMessage<Msg>(std::forward<Args>(args)...));
  }

  template <class Msg, typename... Args>
  bool postAndWait(Args &&... args) {
    return postAndWait(makeMessage<Msg>(std::forward<Args>(args)...));
  }

  template <class Msg>
  void unregisterAllHandlers() {
    unregisterAllHandlers(msgid<Msg>());
  }

  ~Component();

 private:
  std::unique_ptr<struct ComponentDataPrv> d_;
};

namespace this_component {
MAF_EXPORT std::shared_ptr<Component> instance();
MAF_EXPORT std::weak_ptr<Component> ref();
MAF_EXPORT const ComponentID &id();
MAF_EXPORT bool stop();
MAF_EXPORT bool stopped();
MAF_EXPORT bool post(Message msg);
MAF_EXPORT Component::Executor getExecutor();
MAF_EXPORT void unregisterHandler(const HandlerRegID &regid);
MAF_EXPORT void unregisterAllHandlers(const MessageID &regid);

template <class Msg, typename... Args>
static bool post(Args &&... args) {
  return post(makeMessage<Msg>(std::forward<Args>(args)...));
}

template <class Msg>
HandlerRegID onMessage(SpecificMessageHandler<Msg> f) {
  return instance()->onMessage<Msg>(std::move(f));
}

template <class Msg>
void unregisterAllHandlers() {
  unregisterAllHandlers(msgid<Msg>());
}

};  // namespace this_component

}  // namespace messaging
}  // namespace maf
