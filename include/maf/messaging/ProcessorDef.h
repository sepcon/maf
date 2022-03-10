#pragma once

#include <maf/threading/Upcoming.h>

#include <any>
#include <chrono>
#include <functional>
#include <memory>
#include <string>
#include <typeindex>

namespace maf {
namespace messaging {

class Processor;
class MsgConnection;
using ProcessorInstance = std::shared_ptr<Processor>;
using ProcessorRef = std::weak_ptr<Processor>;
using ProcessorID = std::string;
using Message = std::any;
using MessageID = std::type_index;
using MessageProcessingCallback = std::function<void(const Message&)>;
using Execution = std::function<void()>;
using ExecutionTimeout = std::chrono::microseconds;
using ExecutionDeadline = std::chrono::system_clock::time_point;
template <class Msg>
using SpecificMsgProcessingCallback = std::function<void(const Msg&)>;
using EmptyMsgProcessingCallback = std::function<void()>;
using threading::Upcoming;

// -----------------------------------------------------------

template <class Msg>
MessageID msgid();
template <class Msg>
MessageID msgid(Msg&& msg);
template <class SpecificMsg, class... Args>
Message makeMessage(Args&&... args);

template <class Msg>
MessageID msgid() {
  return typeid(Msg);
}

template <class Msg>
MessageID msgid(Msg&& msg) {
  return typeid(std::forward<Msg>(msg));
}

template <class SpecificMsg, class... Args>
Message makeMessage(Args&&... args) {
  return SpecificMsg{std::forward<Args>(args)...};
}

}  // namespace messaging
}  // namespace maf
