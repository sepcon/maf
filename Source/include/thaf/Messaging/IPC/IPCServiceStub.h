#ifndef IPCSERVICESTUB_H
#define IPCSERVICESTUB_H

#include "thaf/Messaging/Message.h"
#include "IPCCommunicator.h"
#include "IPCRequestTracker.h"
#include "IPCInfo.h"
#include <mutex>

namespace thaf {
namespace messaging {
namespace ipc {

class IPCServiceStubHandler
{
public:
    virtual ~IPCServiceStubHandler() = default;
    virtual void onClientRequest(const std::shared_ptr<IPCRequestTracker>& RequestTracker) = 0;

};

class IPCServiceStub : public IPCCommunicator
{
    using IPCMsgPtr = std::shared_ptr<IPCMessage>;
    using IPCRequestTrackerPtr = std::shared_ptr<IPCRequestTracker>;
    using OpID2RequestTrackerMap = stl::MutexContainer<std::map<OpID, std::list<IPCRequestTrackerPtr>>>;
public:
    IPCServiceStub(IPCServiceStubHandler* stubHandler = nullptr);
    ~IPCServiceStub() override;
    void setStubHandler(IPCServiceStubHandler *stubHandler);

    void init(IPCType ipcType, Address myAddr);
    void deinit() override;

    bool replyToRequest(const IPCMsgPtr& msg);
    bool sendStatusUpdate(const IPCMsgPtr& ipcMsg);

private:
    void onIPCMessage(const IPCMsgPtr& msg) override;
    void onStatusChangeRegister(const IPCMsgPtr& msg);
    void onStatusChangeUnregister(const IPCMsgPtr& msg);
    void forwardToStubHandler(const IPCRequestTrackerPtr& clp);

    IPCRequestTrackerPtr saveRequestInfo(const IPCMsgPtr& msg);
    IPCRequestTrackerPtr removeRequestInfo(const IPCMsgPtr &msg);
    void invalidateAndRemoveAllRequestTrackers();

    void saveRegisterInfo(const IPCMsgPtr& msg);
    void removeRegisterInfo(const IPCMsgPtr& msg);
    void removeAllRegisterInfo();
    void removeRegistersOfAddress(const Address& addr);

    void onAbortActionRequest(const IPCMsgPtr& msg);
    void onClientRequest(const IPCMsgPtr& msg);


    IPCServiceStubHandler* _stubHandler;
    RegisteredClientsMap _regEntriesMap;
    OpID2RequestTrackerMap _requestTrackersMap;
    util::IDManager _idMgr;
};

}// ipc
}// messaging
}// thaf
#endif // IPCSERVICESTUB_H
