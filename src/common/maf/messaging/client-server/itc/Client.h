#pragma once

#include "../ClientBase.h"
#include <maf/patterns/Patterns.h>

namespace maf {
namespace messaging {
namespace itc {

class Client : public ClientBase, pattern::Unasignable {
  friend class Server;

public:
  bool start() override { return true; }
  void stop() override {}
  void deinit() override {}
  static std::shared_ptr<Client> instance();
  ActionCallStatus sendMessageToServer(const CSMessagePtr &msg) override;
};
} // namespace itc
} // namespace messaging
} // namespace maf
