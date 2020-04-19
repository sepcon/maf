#pragma once

#include "cs_param.h"

namespace maf {
namespace messaging {

struct ParamTraitBase {

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

  template <class T> static constexpr bool encodable() {
    return std::is_base_of_v<cs_inputbase, T> ||
           std::is_base_of_v<cs_outputbase, T>;
  }


  template <class Message> static constexpr OpIDConst getOperationID() {
    return Message::operationID();
  }

  template <class Message> static OpID getOperationID(Message *msg) {
    if (msg) {
      return OpIDInvalid;
    } else {
      return msg->operationID();
    }
  }

  template <class Message>
  static std::string dump(const std::shared_ptr<Message> &msg) {
    if (msg) {
      return msg->dump();
    } else {
      return "Null";
    }
    //    return "Not Dumpable!";
  }

};
} // namespace messaging
} // namespace maf
