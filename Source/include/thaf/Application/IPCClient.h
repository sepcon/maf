#ifndef IPCCLIENT_H
#define IPCCLIENT_H

#include "thaf/Messaging/IPC/IPCServiceProxy.h"
#include "thaf/Patterns/Patterns.h"
#include "thaf/Utils/Debugging/Debug.h"
#include "IPCMessages.h"

namespace thaf {
using namespace messaging::ipc;
namespace app {

class IPCClient : public pattern::UnCopyable, public ServiceStatusObserver
{
public:
    using RegID = messaging::ipc::RegID;
    template <class DataCarrier>
    using PayloadProcessCallback = std::function<void(const std::shared_ptr<DataCarrier>&)>;

    IPCClient(class AppComponent* comp);
    ~IPCClient();
    void init(IPCType ipcType, Address receiverAddr);
    void stop();

    template<class DataCarrier, std::enable_if_t<std::is_base_of_v<IPCDataCarrier, DataCarrier>, bool> = true>
    RegID sendStatusChangeRegister(messaging::ipc::OpID propertyID, PayloadProcessCallback<DataCarrier> callback);
    void sendStatusChangeUnregister(RegID regID);

    template<class IncomingDataCarrier, std::enable_if_t<std::is_base_of_v<IPCDataCarrier, IncomingDataCarrier>, bool> = true>
    RegID sendActionRequest
        (
            const std::shared_ptr<IPCDataCarrier>&,
            PayloadProcessCallback<IncomingDataCarrier> callback
            );

    template<class IncomingDataCarrier, std::enable_if_t<std::is_base_of_v<IPCDataCarrier, IncomingDataCarrier>, bool> = true>
    bool sendActionRequestSync
        (
            const std::shared_ptr<IPCDataCarrier>& outgoingData,
            PayloadProcessCallback<IncomingDataCarrier> callback,
            unsigned long maxWaitTimeMs = static_cast<unsigned long>(-1)
            );

private:
    void onStatusChanged(ConnectionStatus oldStatus, ConnectionStatus newStatus) override;
    template<class IncomingDataCarrier>
    IPCServiceProxy::MessageHandlerCallback createMsgHandlerCallback(PayloadProcessCallback<IncomingDataCarrier> callback, bool sync = false);
    std::unique_ptr<messaging::ipc::IPCServiceProxy> _serviceProxy;
    class AppComponent* _comp;
};

template<class IncomingDataCarrier>
IPCServiceProxy::MessageHandlerCallback IPCClient::createMsgHandlerCallback(PayloadProcessCallback<IncomingDataCarrier> callback, bool sync)
{
    IPCServiceProxy::MessageHandlerCallback ipcMessageHandlerCB =
            [this, callback, sync](const std::shared_ptr<IPCMessage>& msg){
        auto dataCarrier = IncomingDataCarrier::create();
        if(dataCarrier->getID() == msg->get_operation_id())
        {
            try
            {
                dataCarrier->fromBytes(msg->get_payload());
                if(sync)
                {
                    callback(dataCarrier);
                }
                else
                {
                    _comp->postMessage<CallbackExcMsg>( [dataCarrier, callback] {
                        callback(dataCarrier);
                    });
                }
            }
            catch(const std::exception& e)
            {
                thafErr(e.what());
            }
        }
        else
        {
            thafErr("mismatched of OpID between client[" << dataCarrier->getID() <<
                    "] and server[" << msg->get_operation_id() << "]");
        }
    };
    return ipcMessageHandlerCB;
}


template<class IncomingDataCarrier, std::enable_if_t<std::is_base_of_v<IPCDataCarrier, IncomingDataCarrier>, bool> >
IPCClient::RegID IPCClient::sendActionRequest(const std::shared_ptr<IPCDataCarrier> & outgoingData, PayloadProcessCallback<IncomingDataCarrier> callback)
{
    assert(_comp && _serviceProxy);
    return _serviceProxy->sendRequest(createIPCMsg(OpCode::Request, outgoingData->getID(), outgoingData->toBytes()), createMsgHandlerCallback(callback));
}

template<class IncomingDataCarrier, std::enable_if_t<std::is_base_of_v<IPCDataCarrier, IncomingDataCarrier>, bool> >
bool IPCClient::sendActionRequestSync(const std::shared_ptr<IPCDataCarrier> & outgoingData, PayloadProcessCallback<IncomingDataCarrier> callback, unsigned long maxWaitTimeMs)
{
    assert(_comp && _serviceProxy);
    return _serviceProxy->sendRequestSync
        (
            createIPCMsg(OpCode::RequestSync, outgoingData->getID(), outgoingData->toBytes()),
            createMsgHandlerCallback(callback, true),
            maxWaitTimeMs
            );
}

template<class DataCarrier, std::enable_if_t<std::is_base_of_v<IPCDataCarrier, DataCarrier>, bool> >
IPCClient::RegID IPCClient::sendStatusChangeRegister(messaging::ipc::OpID propertyID, PayloadProcessCallback<DataCarrier> callback)
{
    assert(_comp && _serviceProxy);
    return _serviceProxy->sendStatusChangeRegister(propertyID, createMsgHandlerCallback(std::move(callback)));
}

} // app
} // thaf
#endif // IPCCLIENT_H
