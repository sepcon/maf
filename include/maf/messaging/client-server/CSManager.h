#pragma once

#include "CSTypes.h"
#include "ServiceProviderInterface.h"
#include "ServiceRequesterInterface.h"
#include <maf/export/MafExport_global.h>
#include <maf/patterns/Patterns.h>
#include <memory>

namespace maf {
namespace messaging {


struct CSManagerImpl;
struct Address;

class CSManager : public pattern::Unasignable
{
    std::unique_ptr<CSManagerImpl> _pImpl;
    CSManager();
    ~CSManager();
public:
    MAF_EXPORT static CSManager& instance();

    MAF_EXPORT std::shared_ptr<ServiceRequesterInterface> getServiceRequester(
        const ConnectionType& conntype,
        const Address& serverAddr,
        const ServiceID& sid
        );
    MAF_EXPORT std::shared_ptr<ServiceProviderInterface> getServiceProvider(
        const ConnectionType& conntype,
        const Address& serverAddr,
        const ServiceID& sid
        );
};

} // messaging
} // maf
