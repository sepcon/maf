#pragma once

#include "CSTypes.h"
#include "ServiceProviderIF.h"
#include "ServiceRequesterIF.h"
#include <maf/export/MafExport_global.h>
#include <maf/patterns/Patterns.h>
#include <memory>

namespace maf {
namespace messaging {

struct CSManagerImpl;
struct Address;

class CSManager : public pattern::Unasignable {
  std::unique_ptr<CSManagerImpl> pImpl_;
  CSManager();
  ~CSManager();

public:
  MAF_EXPORT static CSManager &instance() noexcept;

  MAF_EXPORT std::shared_ptr<ServiceRequesterIF>
  getServiceRequester(const ConnectionType &conntype, const Address &serverAddr,
                      const ServiceID &sid) noexcept;
  MAF_EXPORT std::shared_ptr<ServiceProviderIF>
  getServiceProvider(const ConnectionType &conntype, const Address &serverAddr,
                     const ServiceID &sid) noexcept;
};

} // namespace messaging
} // namespace maf
