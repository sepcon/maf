#pragma once

#include "../ServerBase.h"
#include <maf/patterns/Patterns.h>

namespace maf {
namespace messaging {
namespace itc {

class Server : public ServerBase, pattern::Unasignable {
  friend class Client;

public:
  static std::shared_ptr<Server> instance();
  bool start() override { return true; }
  void stop() override {}
  void deinit() override {}
  ActionCallStatus sendMessageToClient(const CSMessagePtr &msg,
                                       const Address &addr = {}) override;
  void notifyServiceStatusToClient(const ServiceID &sid, Availability oldStatus,
                                   Availability newStatus) override;
};

} // namespace itc
} // namespace messaging
} // namespace maf
