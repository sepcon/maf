#ifndef IPCRECEIVER_H
#define IPCRECEIVER_H

#include <maf/messaging/client-server/Connection.h>
#include <maf/messaging/client-server/Address.h>
#include <memory>
#include <list>

namespace maf {
namespace srz { struct ByteArray; }
namespace messaging {
namespace ipc {

class BytesComeObserver
{
public:
    virtual ~BytesComeObserver() = default;
    virtual void onBytesCome(const std::shared_ptr<srz::ByteArray>& bytes) = 0;

private:
    friend class IPCReceiver;
};

class IPCReceiver
{
public:
    virtual ~IPCReceiver() = default;
    virtual bool initConnection(const Address& address, bool isClientMode = false) = 0;
    virtual bool startListening() = 0;
    virtual bool stopListening() = 0;
    virtual bool listening() const = 0;
    virtual const Address& address() const = 0;

    void registerObserver(BytesComeObserver* observer);
    void unregisterObserver(BytesComeObserver* observer);
protected:
    void notifyObervers(const std::shared_ptr<srz::ByteArray>& bytes);
    std::list<BytesComeObserver*> _observers;

};

}// ipc
}// messaging
}// maf
#endif // IPCRECEIVER_H
