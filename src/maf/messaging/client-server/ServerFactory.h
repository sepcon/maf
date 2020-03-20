#pragma once

#include <maf/export/MafExport_global.h>
#include <maf/messaging/client-server/CSTypes.h>
#include <maf/messaging/client-server/ServerIF.h>
#include <maf/patterns/Patterns.h>

namespace maf {
namespace messaging {

class ServerFactoryImpl;

class ServerFactory : public pattern::SingletonObject<ServerFactory> {
  std::unique_ptr<ServerFactoryImpl> pImpl_;

public:
  ~ServerFactory();
  ServerFactory(Invisible) noexcept;

  std::shared_ptr<ServerIF> getServer(const ConnectionType &connectionType,
                                      const Address &addr) noexcept;
};

} // namespace messaging
} // namespace maf
