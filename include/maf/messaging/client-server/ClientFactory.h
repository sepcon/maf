#pragma once

#include "CSTypes.h"
#include "ServerInterface.h"
#include "ClientInterface.h"
#include <maf/patterns/Patterns.h>

namespace maf {
namespace messaging {

class ClientFactoryImpl;
class ClientFactory : public pattern::SingletonObject<ClientFactory>
{
    std::unique_ptr<ClientFactoryImpl> _pImpl;
public:
    std::shared_ptr<ClientInterface> getClient(const ConnectionType& connectionType, const Address& addr);

    ClientFactory(Invisible);
    ~ClientFactory();
};


} // messaging
} // maf
