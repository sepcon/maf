#pragma once

#include <forward_list>

#include "Component.h"
#include "ComponentRequest.h"

namespace maf {
namespace messaging {
namespace details {

using namespace std;

class DuplicateHandlerEx : public runtime_error {
 public:
  DuplicateHandlerEx()
      : runtime_error("Request already handled by other handler") {}
};

template <class Message_>
class MessageHandler {
 protected:
  ComponentRef compref_;
  ConnectionID conid_;

 public:
  using MessageType = Message_;

  explicit MessageHandler(ComponentRef&& compref) : compref_(move(compref)) {}
  MessageHandler(MessageHandler&&) = default;
  MessageHandler& operator=(MessageHandler&&) = default;

  MessageHandler(const MessageHandler&) = delete;
  MessageHandler& operator=(const MessageHandler&) = delete;

  ~MessageHandler() { disconnect(); }

  template <class MessageProcessingCallback_>
  bool connect(MessageProcessingCallback_ processMsg) {
    if (!conid_.valid()) {
      if (auto comp = compref_.lock()) {
        conid_ = comp->connect<MessageType>(move(processMsg));
        return conid_.valid();
      }
    }
    return false;
  }

  void disconnect() {
    if (conid_.valid()) {
      if (auto comp = compref_.lock()) {
        comp->disconnect(conid_);
      }
      conid_.invalidate();
    }
  }
};

template <class Output_, class Input_>
class RequestHandler
    : protected MessageHandler<details::RequestMsg_<Output_, Input_>> {
 public:
  using Output = Output_;
  using Input = Input_;
  using Base = MessageHandler<details::RequestMsg_<Output_, Input_>>;
  using Base::Base;
  using Base::disconnect;
  using Base::operator=;

  template <class RequestHandlerCallback>
  static ConnectionID connect(const ComponentInstance& comp,
                              RequestHandlerCallback&& requestHandlerCallback) {
    using RetType = decltype(requestHandlerCallback(std::declval<Input>()));
    using RequestMsg = details::RequestMsg_<Output, Input>;

    static_assert(
        is_same_v<RetType, Output> || is_constructible_v<Output, RetType>,
        "Return type of RequestHandlerCallback must be same with "
        "Output type of Request handler");

    if (!comp->connected(msgid<RequestMsg>())) {
      auto makeOutputAndRespond =
          [outputMakerCb{std::move(requestHandlerCallback)}](
              const RequestMsg& req) mutable {
            req.onOutputGeneratorCallback(outputMakerCb);
          };

      return comp->connect<RequestMsg>(move(makeOutputAndRespond));
    } else {
      throw DuplicateHandlerEx{};
    }
  }

  template <class OutputMakerCallback>
  bool connect(OutputMakerCallback outputMakerCb) {
    if (this->conid_.valid()) {
      throw DuplicateHandlerEx{};
    }

    if (auto comp = this->compref_.lock()) {
      this->conid_ = connect(comp, move(outputMakerCb));
    }

    return this->conid_.valid();
  }
};

class MessageHandlerGroup {
 public:
  using ConnectionIDs = std::forward_list<ConnectionID>;

  explicit MessageHandlerGroup(ComponentRef&& compref)
      : compref_(std::move(compref)) {}
  ~MessageHandlerGroup() { disconnect(); }

  MessageHandlerGroup() = default;
  MessageHandlerGroup(MessageHandlerGroup&&) = default;
  MessageHandlerGroup& operator=(MessageHandlerGroup&&) = default;

  MessageHandlerGroup(const MessageHandlerGroup&) = delete;
  MessageHandlerGroup& operator=(const MessageHandlerGroup&) = delete;

  bool connectable() const { return compref_.lock() != nullptr; }

  template <class Msg, class ProcessingCallback>
  MessageHandlerGroup& connect(ProcessingCallback callback) {
    if (auto comp = compref_.lock()) {
      addConnectionID(comp->connect<Msg>(std::move(callback)));
    }
    return *this;
  }

  template <class Msg>
  MessageHandlerGroup& disconnect() {
    return disconnect(msgid<Msg>());
  }

  MessageHandlerGroup& disconnect(const MessageID& mid) {
    using namespace std;
    if (auto comp = compref_.lock()) {
      connectionIDs_.remove_if([&mid, &comp](const ConnectionID& cid) {
        if (cid.mid_ == mid) {
          comp->disconnect(cid);
          return true;
        } else {
          return false;
        }
      });
    }
    return *this;
  }

  void disconnect() {
    if (!connectionIDs_.empty()) {
      if (auto comp = compref_.lock()) {
        for (const auto& regid : connectionIDs_) {
          comp->disconnect(regid);
        }
      }
      connectionIDs_.clear();
    }
  }

  template <class Input, class RequestHandlerCallback>
  MessageHandlerGroup& connectRequest(RequestHandlerCallback handle) {
    using Output = decltype(handle(std::declval<Input>()));
    if (auto id = RequestHandler<Output, Input>::connect(this->compref_.lock(),
                                                         move(handle));
        id.valid()) {
      this->addConnectionID(move(id));
    }
    return *this;
  }

  template <class Output, class Input>
  void disconnectRequest() {
    disconnect<RequestMsg_<Output, Input>>();
  }

  template <class Input>
  void disconnectRequest() {
    disconnect<RequestMsg_<void, Input>>();
  }

 protected:
  void addConnectionID(ConnectionID id) {
    connectionIDs_.push_front(std::move(id));
  }
  ConnectionIDs connectionIDs_;
  ComponentRef compref_;
};

class RequestHandlerGroup : protected MessageHandlerGroup {
  using Base = MessageHandlerGroup;

 public:
  using Base::Base;
  using Base::disconnect;
  using Base::operator=;

  template <class Input, class RequestHandlerCallback>
  RequestHandlerGroup& connect(RequestHandlerCallback handle) {
    Base::connectRequest<Input, RequestHandlerCallback>(move(handle));
    return *this;
  }

  template <class Output, class Input>
  void disconnect() {
    Base::disconnectRequest<Output, Input>();
  }

  template <class Input>
  void disconnect() {
    Base::disconnectRequest<void, Input>();
  }
};

}  // namespace details

using details::DuplicateHandlerEx;
using details::MessageHandler;
using details::MessageHandlerGroup;
using details::RequestHandler;
using details::RequestHandlerGroup;

}  // namespace messaging
}  // namespace maf
