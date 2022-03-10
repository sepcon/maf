#pragma once

#include <maf/export/MafExport_global.h>

#include <memory>

#include "ServiceProviderIF.h"
#include "ServiceRequesterIF.h"

namespace maf {
namespace messaging {
namespace csmgmt {

MAF_EXPORT void shutdownAllServers();
MAF_EXPORT void shutdownAllClients();
MAF_EXPORT std::shared_ptr<ServiceRequesterIF> getServiceRequester(
    const ConnectionType &conntype, const Address &serverAddr,
    const ServiceID &sid) noexcept;
MAF_EXPORT std::shared_ptr<ServiceProviderIF> getServiceProvider(
    const ConnectionType &conntype, const Address &serverAddr,
    const ServiceID &sid) noexcept;

}  // namespace csmgmt
}  // namespace messaging
}  // namespace maf
