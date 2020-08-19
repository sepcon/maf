#pragma once

// clang-format off
#include <functional>
#include <memory>
#include <typeindex>
#include <string>
#include <any>
#include <chrono>
// clang-format on

namespace maf {
namespace messaging {

class Component;
struct HandlerRegID;
using ComponentInstance = std::shared_ptr<Component>;
using ComponentRef = std::weak_ptr<Component>;
using ComponentID = std::string;
using Message = std::any;
using MessageID = std::type_index;
using MessageHandler = std::function<void(const Message&)>;
using Execution = std::function<void()>;
using ExecutionTimeout = std::chrono::milliseconds;
using ExecutionDeadline = std::chrono::system_clock::time_point;
template <class Msg>
using SpecificMessageHandler = std::function<void(const Msg&)>;
using DontCareMsgContentHandler = std::function<void()>;

// -----------------------------------------------------------

template <class Msg>
MessageID msgid();
template <class Msg>
MessageID msgid(Msg&& msg);
template <class SpecificMsg, class... Args>
decltype(auto) makeMessage(Args&&... args);

struct HandlerRegID {
  using HandlerID = void*;
  static inline constexpr HandlerID InvalidHandlerID = nullptr;

  bool valid() const {
    return hid_ != InvalidHandlerID && mid_ != msgid<std::nullptr_t>();
  }

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

template <class SpecificMsg, class... Args>
decltype(auto) makeMessage(Args&&... args) {
  using namespace std;
  constexpr bool isMessageConstructible =
      is_trivially_constructible_v<SpecificMsg, Args...> ||
      is_constructible_v<SpecificMsg, Args...>;
  if constexpr (isMessageConstructible) {
    return make_any<SpecificMsg>(forward<Args>(args)...);
  } else {
    return make_any<SpecificMsg>(SpecificMsg{forward<Args>(args)...});
  }
}
}  // namespace messaging
}  // namespace maf
