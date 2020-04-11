#ifndef TYPEIDPARAMTRAIT_H
#define TYPEIDPARAMTRAIT_H

#include "internal/CSShared.h"
#include <type_traits>
#include <typeinfo>

namespace maf {
namespace messaging {

namespace details {

struct void_i {};
struct void_o {};
struct input_base {};
struct output_base {};
template <class Input = void_i, class Output = void_o> struct RequestT {

  static OpIDConst operationID() { return typeid(RequestT).name(); }
  struct input : public Input, input_base {
    using Input::Input;
    static OpIDConst operationID() { return typeid(RequestT).name(); }
  };
  using input_ptr = std::shared_ptr<input>;

  struct output : public Output, output_base {
    static OpIDConst operationID() { return typeid(RequestT).name(); }
  };
  using output_ptr = std::shared_ptr<output>;
};

} // namespace details

struct TypeIDParamTrait {

  template <typename T> static constexpr bool IsStatus = true;

  template <typename T> static constexpr bool IsAttributes = true;

  template <typename T>
  static constexpr bool IsInput = std::is_base_of_v<details::input_base, T>;

  template <typename T>
  static constexpr bool IsOutput = std::is_base_of_v<details::output_base, T>;

  template <typename T> static constexpr bool IsSignal = true;

  template <typename T>
  static constexpr bool IsRequest = true;

  template <typename T> static constexpr bool IsProperty = true;

  template <class T> static constexpr bool encodable() { return true; }

  template <class Message> static constexpr OpIDConst getOperationID() {
    if constexpr (IsInput<Message> || IsOutput<Message> || IsRequest<Message>) {
      return Message::operationID();
    } else {
      return typeid(Message).name();
    }
  }

  template <class Message> static OpID getOperationID(Message *) {
    return getOperationID<Message>();
  }

  template <class Message>
  static std::string dump(const std::shared_ptr<Message> &msg) {
    if (msg) {
      return getOperationID<Message>();
    } else {
      return "Null";
    }
  }

  template <class Message>
  static std::shared_ptr<Message>
  translate(const CSMsgContentBasePtr &csMsgContent,
            TranslationStatus * = nullptr) {
    return std::reinterpret_pointer_cast<Message>(csMsgContent);
  }

  template <class Message>
  static CSMsgContentBasePtr translate(const std::shared_ptr<Message> &msg) {
    static_assert(std::is_copy_constructible_v<Message>,
                  "Message must be copy constructible!");
    if (msg) {
      // for inter-thread communication, the content of message should be
      // cloned instead of sharing by reference/pointer
      return std::reinterpret_pointer_cast<CSMessageContentBase>(
          std::make_shared<Message>(*msg));
    } else {
      return {};
    }
  }
};
} // namespace messaging
} // namespace maf

#endif // TYPEIDPARAMTRAIT_H
