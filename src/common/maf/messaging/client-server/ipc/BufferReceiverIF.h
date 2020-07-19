#pragma once

#include <maf/messaging/client-server/Address.h>
#include <maf/utils/serialization/Buffer.h>

namespace maf {
namespace messaging {
namespace ipc {

class BytesComeObserver {
public:
  virtual ~BytesComeObserver() = default;
  virtual void onBytesCome(srz::Buffer &&bytes) = 0;
};

class BufferReceiverIF {
public:
  virtual ~BufferReceiverIF() = default;
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
