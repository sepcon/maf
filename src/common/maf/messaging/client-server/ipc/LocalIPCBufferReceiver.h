#pragma once

#include "BufferReceiverIF.h"

namespace maf {
namespace messaging {
namespace ipc {
namespace local {

class LocalIPCBufferReceiver : public BufferReceiverIF {
 public:
  LocalIPCBufferReceiver();
  ~LocalIPCBufferReceiver() override;
  bool init(const Address &address) override;
  bool start() override;
  bool running() const override;
  void stop() override;
  void deinit() override;
  const Address &address() const override;
  void setObserver(BytesComeObserver *observer) override;

 private:
  std::unique_ptr<class LocalIPCBufferReceiverImpl> _impl;
};
}  // namespace local
}  // namespace ipc
}  // namespace messaging
}  // namespace maf
