#pragma once

#include "IPCReceiverIF.h"

namespace maf {
namespace messaging {
namespace ipc {

class LocalIPCReceiver : public IPCReceiverIF {
public:
  LocalIPCReceiver();
  ~LocalIPCReceiver() override;
  bool init(const Address &address) override;
  bool start() override;
  bool running() const override;
  void stop() override;
  void deinit() override;
  const Address &address() const override;
  void setObserver(BytesComeObserver *observer) override;

private:
  std::unique_ptr<class LocalIPCReceiverImpl> _impl;
};
} // namespace ipc
} // namespace messaging
} // namespace maf
