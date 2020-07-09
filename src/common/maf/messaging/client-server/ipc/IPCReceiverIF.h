#pragma once

#include <maf/messaging/client-server/Address.h>
#include <maf/utils/serialization/ByteArray.h>

namespace maf {
namespace messaging {
namespace ipc {

class BytesComeObserver {
public:
  virtual ~BytesComeObserver() = default;
  virtual void onBytesCome(srz::ByteArray &&bytes) = 0;
};

class IPCReceiverIF {
public:
  virtual ~IPCReceiverIF() = default;
  virtual bool init(const Address &address) = 0;
  virtual bool start() = 0;
  virtual void stop() = 0;
  virtual void deinit() = 0;
  virtual bool running() const = 0;
  virtual const Address &address() const = 0;
  virtual void setObserver(BytesComeObserver *observer) = 0;
};

} // namespace ipc
} // namespace messaging
} // namespace maf
