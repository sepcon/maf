#pragma once

#include "Address.h"

namespace thaf {
namespace messaging {

class DomainUser
{
public:
    Address::Port _servingPort;
    inline Address::Port servicingPort() const;
    inline void setServingPort(const Address::Port &servicingPort);
};

Address::Port DomainUser::servicingPort() const
{
    return _servingPort;
}

void DomainUser::setServingPort(const Address::Port &servicingPort)
{
    _servingPort = servicingPort;
}

}
}
