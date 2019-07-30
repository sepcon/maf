#include "thaf/messaging/client-server/DomainController.h"
#include "thaf/messaging/client-server/interfaces/DomainUser.h"
#include "thaf/utils/debugging/Debug.h"

namespace thaf {
namespace messaging {

bool DomainController::registerUser(const DomainUserPtr &user)
{
    auto lock(_registeredUsers.pa_lock());
    auto it = _registeredUsers->find(user->servicingPort());
    if(it == _registeredUsers->end())
    {
        thafInfo("New user registered to service on port " << user->servicingPort());
        _registeredUsers->insert(std::make_pair(user->servicingPort(), user));
        return true;
    }
    else
    {
        thafErr("The servicing port is already used by a user");
        return false;
    }
}

bool DomainController::unregisterUser(DomainController::ServicingPort port)
{
    auto lock(_registeredUsers.pa_lock());
    return _registeredUsers->erase(port) != 0;
}

bool DomainController::unregisterUser(const DomainUserPtr &user)
{
    return unregisterUser(user->servicingPort());
}

DomainController::DomainUserPtr DomainController::getUser(DomainController::ServicingPort port)
{
    auto lock(_registeredUsers.pa_lock());
    auto it = _registeredUsers->find(port);
    if(it == _registeredUsers->end())
    {
        return it->second;
    }
    else
    {
        return {};
    }
}

const DomainController::DomainName &DomainController::domainName() const
{
    return _domainName;
}

void DomainController::setDomainName(DomainName domainName)
{
    _domainName = std::move(domainName);
}




}
}
