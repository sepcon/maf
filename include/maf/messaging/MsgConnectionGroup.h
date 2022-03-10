#pragma once

#include "Processor.h"

namespace maf::messaging {
class MsgConnectionGroup {
 public:
  MsgConnectionGroup() = default;
  ~MsgConnectionGroup() { disconnect(); }

  void disconnect() {
    for (auto& con : connections_) {
      con.disconnect();
    }
  }

  MsgConnectionGroup& operator<<(MsgConnection con) {
    return add(std::move(con));
  }

  MsgConnectionGroup& add(MsgConnection con) {
    connections_.push_back(std::move(con));
    return *this;
  }

 private:
  std::vector<MsgConnection> connections_;
};
}  // namespace maf::messaging
