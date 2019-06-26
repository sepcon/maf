#pragma once

#include "NamedPipeReceiverBase.h"

namespace thaf {
namespace messaging {
namespace ipc {


class NamedPipeReceiver : public NamedPipeReceiverBase
{
public:
    NamedPipeReceiver() : _pipeHandle(nullptr){}
    bool stopListening() override;

private:
    void listningThreadFunction() override;
    std::atomic<void*> _pipeHandle;
};
}
}
}
