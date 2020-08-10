#pragma once

// clang-format off
#include <functional>
#include <memory>
#include <typeindex>
#include <string>
#include <any>
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

template <class Msg>
using SpecificMessageHandler = std::function<void(const Msg&)>;

// -----------------------------------------------------------
struct HandlerRegID {
  using HandlerID = void*;
  static inline constexpr HandlerID InvalidHandlerID = nullptr;

  bool valid() const {
    return hid_ != InvalidHandlerID && mid_ != typeid(std::nullptr_t);
  }

  HandlerID hid_ = InvalidHandlerID;
  MessageID mid_ = typeid(std::nullptr_t);
};

}  // namespace messaging
}  // namespace maf
