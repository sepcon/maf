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
using ComponentInstance = std::shared_ptr<Component>;
using ComponentRef = std::weak_ptr<Component>;
using ComponentID = std::string;
using ComponentMessage = std::any;
using ComponentMessageID = std::type_index;
using GenericMsgHandlerFunction = std::function<void(ComponentMessage)>;
using Execution = std::function<void()>;

template <class Msg>
using ComponentMessageHandlerFunction = std::function<void(Msg)>;

}  // namespace messaging
}  // namespace maf
