#pragma once

#include "Address.h"
#include "thaf/utils/cppextension/SyncObject.h"
#include "thaf/patterns/Patterns.h"

namespace thaf {
namespace messaging {

class DomainUser;

class DomainController
{
public:
    using DomainUserPtr = std::shared_ptr<DomainUser>;
    using DomainName = Address::Name;
    using ServicingPort = Address::Port;
    bool registerUser(const DomainUserPtr& user);
    bool unregisterUser(ServicingPort port);
    bool unregisterUser(const DomainUserPtr& user);
    DomainUserPtr getUser(ServicingPort port);
    const DomainName& domainName() const;
    void setDomainName(DomainName domainName);
private:
    DomainName _domainName;
    nstl::SyncObject<std::map<ServicingPort, DomainUserPtr>> _registeredUsers;
};
}
}
