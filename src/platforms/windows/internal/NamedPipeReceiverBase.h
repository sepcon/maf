#pragma once

#include "PipeShared.h"
#include <maf/messaging/client-server/ipc/IPCReceiver.h>
#include <maf/logging/Logger.h>
#include <thread>
#include <windows.h>
#include <string>
#include <atomic>

namespace maf { using logging::Logger;
namespace messaging {
namespace ipc {


class NamedPipeReceiverBase : public IPCReceiver
{
public:
    NamedPipeReceiverBase() :  _stopped(true){}
    ~NamedPipeReceiverBase() override
    {
        if(!_stopped)
        {
            stopListening();
        }

        if(_workerThread.joinable())
        {
            _workerThread.join();
        }
    }
    bool initConnection(const Address& address, bool isClientMode = false) override
    {
        _isClient = isClientMode;
        if(isClientMode)
        {
            static std::atomic<uint16_t> receiverCount(0);
            receiverCount += 1;
            uint16_t randomPort = receiverCount;
            _myaddr = Address(address.name() + std::to_string(GetCurrentProcessId()), randomPort);
        }
        else
        {
            _myaddr = std::move(address);
        }
        _pipeName = constructPipeName(_myaddr);
        return true;
    }
    bool startListening() override
    {
        if(!listening())
        {
            _stopped.store(false, std::memory_order_release);
            _workerThread = std::thread { &NamedPipeReceiverBase::listningThreadFunction, this };
        }
        return true;
    }
    bool stopListening() override
    {
        _stopped.store(true, std::memory_order_release);
        waitForWorkerThreadToStop();
        return false;
    }
    bool listening() const override
    {
        return !_stopped.load(std::memory_order_acquire);
    }
    const Address& address() const override
    {
        return _myaddr;
    }

protected:
    virtual void listningThreadFunction() {
        Logger::warn("listningThreadFunction must be overridden by derived class");
    }
    void waitForWorkerThreadToStop()
    {
        if(_workerThread.joinable())
        {
            _workerThread.join();
        }
    }
    std::string _pipeName;
    std::thread _workerThread;
    Address _myaddr;
    std::atomic_bool _stopped;
    bool _isClient;
};

}
}
}
