#pragma once

#include <maf/messaging/client-server/ClientIF.h>
#include <maf/patterns/Patterns.h>

namespace maf {
namespace messaging {

class ClientFactoryImpl;
class ClientFactory : public pattern::SingletonObject<ClientFactory> {
  std::unique_ptr<ClientFactoryImpl> _pImpl;

 public:
  ~ClientFactory();
  ClientFactory(Invisible);
  std::shared_ptr<ClientIF> getClient(const ConnectionType &connectionType,
                                      const Address &addr);
};

}  // namespace messaging
}  // namespace maf
