#pragma once

#include "cs_param.h"

namespace maf {
namespace messaging {
namespace details {

MC_MAF_DEFINE_HAS_METHOD_CHECK_S(dump)

template <class Message, typename = void>
struct __Dumper {
  static std::string dump(const Message&) { return typeid(Message).name(); }
};
template <class Message>
struct __Dumper<Message,
                std::enable_if_t<has_dump_method<std::decay_t<Message>,
                                                 std::string(int) const>::value,
                                 void>> {
  static std::string dump(const Message& msg) { return msg.dump(0); }
};

}  // namespace details

struct ParamTraitBase {
  template <class T>
  static constexpr bool encodable() {
    return std::is_base_of_v<cs_inputbase, T> ||
           std::is_base_of_v<cs_outputbase, T>;
  }

  template <class Message>
  static constexpr OpIDConst getOperationID() {
    return Message::operationID();
  }

  template <class Message>
  static OpID getOperationID(const Message* msg) {
    if (msg) {
      return OpIDInvalid;
    } else {
      return msg->operationID();
    }
  }

  template <typename Input, typename Output>
  static bool isSameOpID(const Input* input, const Output* output) {
    return getOperationID(input) == getOperationID(output);
  }

  template <class Message>
  static std::string dump(const std::shared_ptr<Message>& msg) {
    if (msg) {
      return dump(*msg);
    } else {
      return "null";
    }
  }

  template <class Message>
  static auto dump(const Message& msg) {
    return details::__Dumper<Message>::dump(msg);
  }

  template <typename T>
  static constexpr bool IsStatus = std::is_base_of_v<cs_status, T>;

  template <typename T>
  static constexpr bool IsAttributes = std::is_base_of_v<cs_attributes, T>;

  template <typename T>
  static constexpr bool IsInput = std::is_base_of_v<cs_input, T>;

  template <typename T>
  static constexpr bool IsOutput = std::is_base_of_v<cs_output, T>;

  template <typename T>
  static constexpr bool IsSignal = std::is_base_of_v<cs_signal, T>;

  template <typename T>
  static constexpr bool IsRequest = std::is_base_of_v<cs_request, T>;

  template <typename T>
  static constexpr bool IsProperty = std::is_base_of_v<cs_property, T>;

  template <typename Input, typename Output>
  static constexpr bool IsSameOpID =
      getOperationID<Output>() == getOperationID<Input>();
};
}  // namespace messaging
}  // namespace maf
