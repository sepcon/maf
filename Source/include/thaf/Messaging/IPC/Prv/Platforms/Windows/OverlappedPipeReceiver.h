#pragma once

#include "NamedPipeReceiverBase.h"
#include "windows.h"

namespace thaf {
namespace messaging {
namespace ipc {

struct PipeInstance
{
    srz::ByteArray ba;
    OVERLAPPED oOverlap;
    HANDLE hPipeInst;
	bool fPendingIO;
};

class OverlappedPipeReceiver : public NamedPipeReceiverBase
{
public:
    using PipeInstances = std::vector<std::unique_ptr<PipeInstance>>;
    using Handles = std::vector<HANDLE>;
    OverlappedPipeReceiver();
    bool stopListening() override;

private:
    bool initPipes();
    void listningThreadFunction() override;
    void disconnectAndReconnect(size_t index);
    bool readBytesOnPipe(size_t index);
    PipeInstances _pipeInstances;
    Handles _hEvents;

};
} // ipc
} // messaging
} // thaf
