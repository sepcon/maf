#pragma once

#include <maf/export/MafExport_global.h>
#include <maf/logging/Logger.h>
#include <maf/patterns/Patterns.h>
#include <maf/utils/ExecutorIF.h>

#include <future>

#include "ProcessorDef.h"

namespace maf {
namespace messaging {

class MAF_EXPORT MsgConnection {
 public:
  MsgConnection() = default;
  MsgConnection(void *);
  MsgConnection(MsgConnection &&) noexcept;
  MsgConnection &operator=(MsgConnection &&) noexcept;
  ~MsgConnection();

  MsgConnection(const MsgConnection &) = delete;
  MsgConnection &operator=(const MsgConnection &) = delete;

  bool connected() const;
  void disconnect();
  void reconnect();
  void *_ = nullptr;
};

class Processor final : pattern::Unasignable,
                        public std::enable_shared_from_this<Processor> {
  MAF_EXPORT Processor(ProcessorID id);

 public:
  using ThreadFunction = std::function<void()>;
  using Executor = std::shared_ptr<util::ExecutorIF>;
  using CompleteSignal = Upcoming<void>;

  MAF_EXPORT static ProcessorInstance create(ProcessorID id = {});
  MAF_EXPORT static ProcessorInstance findProcessor(const ProcessorID &id);
  MAF_EXPORT const ProcessorID &id() const noexcept;
  MAF_EXPORT void run(ThreadFunction threadInit = {},
                      ThreadFunction threadDeinit = {});
  MAF_EXPORT void runFor(ExecutionTimeout duration);
  MAF_EXPORT void runUntil(ExecutionDeadline deadline);
  MAF_EXPORT bool runOnceFor(ExecutionTimeout duration);
  MAF_EXPORT bool runOnceUntil(ExecutionDeadline deadline);
  MAF_EXPORT void stop();
  MAF_EXPORT bool stopped() const;
  MAF_EXPORT bool post(Message msg);
  MAF_EXPORT CompleteSignal waitablePost(Message msg);
  MAF_EXPORT bool connected(const MessageID &mid) const;
  MAF_EXPORT bool executeAsync(Execution exec);
  MAF_EXPORT bool execute(Execution exec);
  MAF_EXPORT CompleteSignal waitableExecute(Execution exec);
  MAF_EXPORT Executor getExecutor();
  MAF_EXPORT Executor getAsyncExecutor();
  MAF_EXPORT Executor getBlockingExecutor();
  MAF_EXPORT Execution willExecuteOnThis(Execution exec);
  MAF_EXPORT Execution willAsyncExecuteOnThis(Execution exec);
  MAF_EXPORT Execution willBlockingExecuteOnThis(Execution exec);

  MAF_EXPORT MsgConnection
  connect(const MessageID &msgid, MessageProcessingCallback processMsgCallback);
  MAF_EXPORT void disconnect(const MessageID &msgid);
  MAF_EXPORT size_t pendingCout() const;

  template <class Msg>
  bool connected() const;

  template <class Msg>
  MsgConnection connect(SpecificMsgProcessingCallback<Msg> f);

  template <class Msg>
  MsgConnection connect(EmptyMsgProcessingCallback f);

  template <class Msg, typename... Args>
  bool post(Args &&... args);

  template <class Msg, typename... Args>
  CompleteSignal waitablePost(Args &&... args);

  template <class Msg>
  void disconnect();

  ~Processor();

 private:
  std::unique_ptr<struct ProcessorDataPrv> d_;
};

MAF_EXPORT ProcessorInstance makeProcessor(ProcessorID id = {});

namespace this_processor {
using CompleteSignal = Processor::CompleteSignal;
using Executor = Processor::Executor;
MAF_EXPORT std::shared_ptr<Processor> instance();
MAF_EXPORT std::weak_ptr<Processor> ref();
MAF_EXPORT const ProcessorID &id();
MAF_EXPORT bool stop();
MAF_EXPORT bool stopped();
MAF_EXPORT bool post(Message msg);
MAF_EXPORT Processor::CompleteSignal waitablePost(Message msg);
MAF_EXPORT bool executeAsync(Execution exec);
MAF_EXPORT bool execute(Execution exec);
MAF_EXPORT CompleteSignal waitableExecute(Execution exec);
MAF_EXPORT Executor getAsyncExecutor();
MAF_EXPORT Executor getExecutor();
MAF_EXPORT Executor getWaitableExecutor();
MAF_EXPORT Execution willExecuteOnThis(Execution exec);
MAF_EXPORT Execution willAsyncExecuteOnThis(Execution exec);
MAF_EXPORT Execution willBlockingExecuteOnThis(Execution exec);
MAF_EXPORT void disconnect(const MessageID &regid);

template <class Msg, typename... Args>
bool post(Args &&...args) {
  return post(makeMessage<Msg>(std::forward<Args>(args)...));
}

template <class Msg>
MsgConnection connect(SpecificMsgProcessingCallback<Msg> f) {
  return instance()->connect<Msg>(std::move(f));
}

template <class Msg>
void disconnect() {
  disconnect(msgid<Msg>());
}

}  // namespace this_processor

template <class Msg>
bool Processor::connected() const {
  return connected(msgid<Msg>());
}

template <class Msg>
MsgConnection Processor::connect(SpecificMsgProcessingCallback<Msg> f) {
  using namespace std;
  auto translatorCallback = [callback = move(f),
                             this](const Message &genericMsg) {
    try {
      callback(any_cast<const Msg &>(genericMsg));
    } catch (const bad_any_cast &) {
      MAF_LOGGER_ERROR("Failed to CAST msg to type of ", msgid<Msg>().name());
    }
  };

  return connect(msgid<Msg>(), move(translatorCallback));
}

template <class Msg>
MsgConnection Processor::connect(EmptyMsgProcessingCallback f) {
  return connect(msgid<Msg>(), [f{std::move(f)}](const auto &) { f(); });
}

template <class Msg, typename... Args>
bool Processor::post(Args &&...args) {
  return post(makeMessage<Msg>(std::forward<Args>(args)...));
}

template <class Msg, typename... Args>
Processor::CompleteSignal Processor::waitablePost(Args &&...args) {
  return waitablePost(makeMessage<Msg>(std::forward<Args>(args)...));
}

template <class Msg>
void Processor::disconnect() {
  disconnect(msgid<Msg>());
}

}  // namespace messaging
}  // namespace maf
