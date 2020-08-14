#pragma once

#include <forward_list>

#include "Component.h"

namespace maf {
namespace messaging {

class MessageHandlerManager {
 public:
  using RegID = HandlerRegID;
  using RegIDList = std::forward_list<RegID>;

  explicit MessageHandlerManager(ComponentRef compref)
      : compref_(std::move(compref)) {}
  ~MessageHandlerManager() { unregisterAll(); }

  MessageHandlerManager() = default;
  MessageHandlerManager(MessageHandlerManager&&) = default;
  MessageHandlerManager& operator=(MessageHandlerManager&&) = default;

  MessageHandlerManager(const MessageHandlerManager&) = delete;
  MessageHandlerManager& operator=(const MessageHandlerManager&) = delete;

  MessageHandlerManager& operator<<(RegID regID) {
    if (auto comp = compref_.lock()) {
      regids_.push_front(std::move(regID));
    }
    return *this;
  }

  template <class Msg>
  MessageHandlerManager& onMessage(SpecificMessageHandler<Msg> f) {
    if (auto comp = compref_.lock()) {
      regids_.push_front(comp->onMessage<Msg>(std::move(f)));
    }
    return *this;
  }

  template <class Msg>
  MessageHandlerManager& unregisterHandler() {
    return unregisterHandler(msgid<Msg>());
  }

  MessageHandlerManager& unregisterHandler(const MessageID& mid) {
    using namespace std;
    if (auto comp = compref_.lock()) {
      auto beg = begin(regids_);
      auto ed = end(regids_);
      auto prevRemoved = regids_.before_begin();
      while (beg != ed) {
        if (beg->mid_ == mid) {
          comp->unregisterHandler(*beg);
          beg = regids_.erase_after(prevRemoved);
        } else {
          prevRemoved = beg++;
        }
      };
    }
    return *this;
  }

  void unregisterAll() {
    if (!regids_.empty()) {
      if (auto comp = compref_.lock()) {
        for (const auto& regid : regids_) {
          comp->unregisterHandler(regid);
        }
      }
      regids_.clear();
    }
  }

 private:
  RegIDList regids_;
  ComponentRef compref_;
};

}  // namespace messaging
}  // namespace maf
