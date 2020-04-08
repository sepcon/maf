#pragma once

#include "IPCReceiver.h"

namespace maf {
namespace messaging {
namespace ipc {

class LocalIPCReceiver : public IPCReceiver {
public:
  LocalIPCReceiver();
  ~LocalIPCReceiver() override;
  bool initConnection(const Address &address,
                      bool isClientMode = false) override;
  bool startListening() override;
  bool stopListening() override;
  bool listening() const override;
  const Address &address() const override;
  void registerObserver(BytesComeObserver *observer) override;

private:
  std::unique_ptr<class LocalIPCReceiverImpl> _impl;
};
} // namespace ipc
} // namespace messaging
} // namespace maf
