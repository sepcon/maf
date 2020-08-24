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

class Component;
struct ConnectionID;
using ComponentInstance = std::shared_ptr<Component>;
using ComponentRef = std::weak_ptr<Component>;
using ComponentID = std::string;
using Message = std::any;
using MessageID = std::type_index;
using MessageProcessingCallback = std::function<void(const Message&)>;
using Execution = std::function<void()>;
using ExecutionTimeout = std::chrono::milliseconds;
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
decltype(auto) makeMessage(Args&&... args);

struct ConnectionID {
  using HandlerID = void*;
  static inline constexpr HandlerID InvalidHandlerID = nullptr;

  bool valid() const { return hid_ != InvalidHandlerID; }
  void invalidate() { hid_ = InvalidHandlerID; }

  HandlerID hid_ = InvalidHandlerID;
  MessageID mid_ = msgid<std::nullptr_t>();
};

template <class Msg>
MessageID msgid() {
  return typeid(Msg);
}

template <class Msg>
MessageID msgid(Msg&& msg) {
  return typeid(std::forward<Msg>(msg));
}

template <class SpecificMsg, class Arg0, class... Args>
decltype(auto) makeMessage(Arg0&& arg0, Args&&... args) {
  using namespace std;
  constexpr bool isMessageConstructible =
      is_trivially_constructible_v<SpecificMsg, Arg0, Args...> ||
      is_constructible_v<SpecificMsg, Arg0, Args...>;
  if constexpr (isMessageConstructible) {
    return make_any<SpecificMsg>(forward<Arg0>(arg0), forward<Args>(args)...);
  } else {
    any a = SpecificMsg{forward<Arg0>(arg0), forward<Args>(args)...};
    return a;
  }
}
template <class SpecificMsg>
decltype(auto) makeMessage() {
  std::any a = SpecificMsg{};
  return a;
}

}  // namespace messaging
}  // namespace maf
