#ifndef IPCRECEIVER_H
#define IPCRECEIVER_H

#include <maf/messaging/client-server/Address.h>
#include <maf/utils/serialization/ByteArray.h>
#include <memory>

namespace maf {
namespace messaging {
namespace ipc {

class BytesComeObserver {
public:
  virtual ~BytesComeObserver() = default;
  virtual void onBytesCome(srz::ByteArray &&bytes) = 0;
};

class IPCReceiver {
public:
  virtual ~IPCReceiver() = default;
  virtual bool initConnection(const Address &address,
                              bool isClientMode = false) = 0;
  virtual bool startListening() = 0;
  virtual bool stopListening() = 0;
  virtual bool listening() const = 0;
  virtual const Address &address() const = 0;
  virtual void registerObserver(BytesComeObserver *observer) = 0;
};

} // namespace ipc
} // namespace messaging
} // namespace maf
#endif // IPCRECEIVER_H
