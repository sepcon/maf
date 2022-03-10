#pragma once

#include <maf/messaging/client-server/ServerIF.h>

namespace maf {
namespace messaging {

class ServerFactoryImpl;
class ServerFactory {
  std::unique_ptr<ServerFactoryImpl> pImpl_;

 public:
  ~ServerFactory();
  ServerFactory();

  std::shared_ptr<ServerIF> getServer(const ConnectionType &connectionType,
                                      const Address &addr) noexcept;
  void close();
};

}  // namespace messaging
}  // namespace maf
