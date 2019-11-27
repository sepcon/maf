#pragma once

#include "CSTypes.h"
#include "ServerInterface.h"
#include <maf/export/MafExport_global.h>
#include <maf/patterns/Patterns.h>


namespace maf {
namespace messaging {

class ServerFactoryImpl;

class ServerFactory : public pattern::SingletonObject<ServerFactory>
{
    std::unique_ptr<ServerFactoryImpl> _pImpl;
public:
    ~ServerFactory();
    ServerFactory(Invisible);

    std::shared_ptr<ServerInterface> getServer(
        const ConnectionType& connectionType,
        const Address& addr
        );
};


} // messaging
} // maf

