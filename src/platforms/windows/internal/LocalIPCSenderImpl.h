#pragma once

#include "NamedPipeSenderBase.h"

namespace maf {
namespace messaging {
namespace ipc {


class  LocalIPCSenderImpl: public NamedPipeSenderBase
{
public:
    LocalIPCSenderImpl();
    ~LocalIPCSenderImpl() override;
    ActionCallStatus send(const maf::srz::ByteArray &ba, const Address& destination = Address::INVALID_ADDRESS) override;

private:
    OVERLAPPED _oOverlap;
};

}}}
