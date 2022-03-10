#include "Client.h"

#include <maf/patterns/Patterns.h>

#include "../ClientBase.h"
#include "Server.h"

namespace maf {
namespace messaging {
namespace itc {

class Client : public ClientBase, pattern::Unasignable {
 public:
  bool start() override { return true; }
  void stop() override {}
  void deinit() override {}
  static std::shared_ptr<Client> instance();
  ActionCallStatus sendMessageToServer(const CSMessagePtr &msg) override;
};

static auto server() { return makeServer(); }
std::shared_ptr<Client> Client::instance() {
  static auto instance_ = std::make_shared<Client>();
  return instance_;
}

ActionCallStatus Client::sendMessageToServer(const CSMessagePtr &msg) {
  msg->setSourceAddress(
      Address{"", 0});  // TODO: later must be validated by validator
  if (server()->onIncomingMessage(msg)) {
    return ActionCallStatus::Success;
  } else {
    return ActionCallStatus::ReceiverUnavailable;
  }
}

std::shared_ptr<ClientIF> makeClient() { return Client::instance(); }

}  // namespace itc
}  // namespace messaging
}  // namespace maf
