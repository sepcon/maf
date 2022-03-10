#pragma once

#include <maf/messaging/client-server/ClientIF.h>

namespace maf {
namespace messaging {

class ClientFactoryImpl;
class ClientFactory {
  std::unique_ptr<ClientFactoryImpl> _pImpl;

 public:
  ClientFactory();
  ~ClientFactory();
  std::shared_ptr<ClientIF> getClient(const ConnectionType &connectionType,
                                      const Address &addr);
  void close();
};

}  // namespace messaging
}  // namespace maf
