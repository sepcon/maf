#pragma once

#include "CSTypes.h"
#include "ServiceProviderIF.h"
#include "ServiceRequesterIF.h"
#include "Address.h"
#include <maf/export/MafExport_global.h>
#include <maf/patterns/Patterns.h>
#include <memory>

namespace maf {
namespace messaging {
namespace csmanagement {


MAF_EXPORT std::shared_ptr<ServiceRequesterIF>
getServiceRequester(const ConnectionType &conntype, const Address &serverAddr,
                    const ServiceID &sid) noexcept;
MAF_EXPORT std::shared_ptr<ServiceProviderIF>
getServiceProvider(const ConnectionType &conntype, const Address &serverAddr,
                   const ServiceID &sid) noexcept;

} // namespace csmanagement
} // namespace messaging
} // namespace maf
