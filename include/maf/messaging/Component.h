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
  MAF_EXPORT Component(ComponentID id);

 public:
  using ThreadFunction = std::function<void()>;
  using Executor = std::shared_ptr<ExecutorIF>;
  using MessageHandledSignal = Upcoming<void>;

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
  MAF_EXPORT MessageHandledSignal send(Message msg);
  MAF_EXPORT bool connected(const MessageID &mid) const;
  MAF_EXPORT bool execute(Execution exec);
  MAF_EXPORT Executor getExecutor();
  MAF_EXPORT ConnectionID connect(const MessageID &msgid,
                                  MessageProcessingCallback processMessage);
  MAF_EXPORT void disconnect(const ConnectionID &regid);
  MAF_EXPORT void disconnect(const MessageID &msgid);
  MAF_EXPORT size_t pendingCout() const;

  template <class Msg>
  bool connected() const;

  template <class Msg>
  ConnectionID connect(SpecificMsgProcessingCallback<Msg> f);

  template <class Msg>
  ConnectionID connect(EmptyMsgProcessingCallback f);

  template <class Msg, typename... Args>
  bool post(Args &&... args);

  template <class Msg, typename... Args>
  MessageHandledSignal send(Args &&... args);

  template <class Msg>
  void disconnect();

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
MAF_EXPORT void disconnect(const ConnectionID &regid);
MAF_EXPORT void disconnect(const MessageID &regid);

template <class Msg, typename... Args>
static bool post(Args &&... args) {
  return post(makeMessage<Msg>(std::forward<Args>(args)...));
}

template <class Msg>
ConnectionID connect(SpecificMsgProcessingCallback<Msg> f) {
  return instance()->connect<Msg>(std::move(f));
}

template <class Msg>
void disconnect() {
  disconnect(msgid<Msg>());
}

};  // namespace this_component

template <class Msg>
bool Component::connected() const {
  return connected(msgid<Msg>());
}

template <class Msg>
ConnectionID Component::connect(SpecificMsgProcessingCallback<Msg> f) {
  using namespace std;
  auto translatorCallback = [callback = move(f),
                             this](const Message &genericMsg) {
    try {
      callback(any_cast<const Msg &>(genericMsg));
    } catch (const bad_any_cast &) {
      MAF_LOGGER_ERROR("Failed to CAST msg to type of ", msgid<Msg>().name());
    } catch (const exception &e) {
      MAF_LOGGER_FATAL("EXCEPTION when handling message ", msgid<Msg>().name(),
                       ": ", e.what());
      throw;
    }
  };

  return connect(msgid<Msg>(), move(translatorCallback));
}

template <class Msg>
ConnectionID Component::connect(EmptyMsgProcessingCallback f) {
  return connect(msgid<Msg>(), [f{std::move(f)}](const auto &) { f(); });
}

template <class Msg, typename... Args>
bool Component::post(Args &&... args) {
  return post(makeMessage<Msg>(std::forward<Args>(args)...));
}

template <class Msg, typename... Args>
Component::MessageHandledSignal Component::send(Args &&... args) {
  return send(makeMessage<Msg>(std::forward<Args>(args)...));
}

template <class Msg>
void Component::disconnect() {
  disconnect(msgid<Msg>());
}

}  // namespace messaging
}  // namespace maf
