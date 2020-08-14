#pragma once

#include <maf/messaging/client-server/CSShared.h>

#include <type_traits>
#include <typeinfo>

namespace maf {
namespace messaging {

namespace details {

struct void_i {};
struct void_o {};
struct input_base {};
struct output_base {};

#define MAF_CS_SIMPLE_REQUEST(RequestName, Input, Output)                 \
  struct RequestName##Request {                                           \
    static constexpr OpIDConst operationID() {                            \
      return #RequestName #Input #Output;                                 \
    }                                                                     \
    struct input : public Input, maf::messaging::details::input_base {    \
      using Input::Input;                                                 \
      static constexpr maf::messaging::OpIDConst operationID() {          \
        return RequestName##Request::operationID();                       \
      }                                                                   \
    };                                                                    \
    using input_ptr = std::shared_ptr<input>;                             \
    template <typename... Args>                                           \
    static input_ptr make_input(Args &&... args) {                        \
      return std::make_shared<input>(std::forward<Args>(args)...);        \
    }                                                                     \
                                                                          \
    struct output : public Output, maf::messaging::details::output_base { \
      using Output::Output;                                               \
      static constexpr maf::messaging::OpIDConst operationID() {          \
        return RequestName##Request::operationID();                       \
      }                                                                   \
    };                                                                    \
    using output_ptr = std::shared_ptr<output>;                           \
    template <typename... Args>                                           \
    static output_ptr make_output(Args &&... args) {                      \
      return std::make_shared<output>(std::forward<Args>(args)...);       \
    }                                                                     \
  };

}  // namespace details

struct TypeIDParamTrait {
  template <typename T>
  static constexpr bool IsStatus = true;

  template <typename T>
  static constexpr bool IsAttributes = true;

  template <typename T>
  static constexpr bool IsInput = std::is_base_of_v<details::input_base, T>;

  template <typename T>
  static constexpr bool IsOutput = std::is_base_of_v<details::output_base, T>;

  template <typename T>
  static constexpr bool IsSignal = true;

  template <typename T>
  static constexpr bool IsRequest = true;

  template <typename T>
  static constexpr bool IsProperty = true;

  template <class T>
  static constexpr bool encodable() {
    return true;
  }

  template <class Message>
  static constexpr OpIDConst getOperationID() {
    if constexpr (IsInput<Message> || IsOutput<Message> || IsRequest<Message>) {
      return Message::operationID();
    } else {
      return typeid(Message).name();
    }
  }

  template <class Message>
  static OpID getOperationID(Message *) {
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
  static std::shared_ptr<Message> translate(const CSPayloadIFPtr &csMsgContent,
                                            TranslationStatus * = nullptr) {
    return std::reinterpret_pointer_cast<Message>(csMsgContent);
  }

  template <class Message>
  static CSPayloadIFPtr translate(const std::shared_ptr<Message> &msg) {
    static_assert(std::is_copy_constructible_v<Message>,
                  "Message must be copy constructible!");
    if (msg) {
      // for inter-thread communication, the content of message should be
      // cloned instead of sharing by reference/pointer
      return std::reinterpret_pointer_cast<CSMsgPayloadIF>(
          std::make_shared<Message>(*msg));
    } else {
      return {};
    }
  }
};
}  // namespace messaging
}  // namespace maf
