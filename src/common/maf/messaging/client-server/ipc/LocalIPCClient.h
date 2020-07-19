#pragma once

#include <future>
#include <thread>

#include "../ClientBase.h"
#include "BufferReceiverIF.h"
#include "IPCTypes.h"

namespace maf {
namespace messaging {
namespace ipc {

class BufferSenderIF;
class BufferReceiverIF;

namespace local {

class LocalIPCClient : public ClientBase, public BytesComeObserver {
 public:
  LocalIPCClient();
  ~LocalIPCClient() override;

  bool init(const Address &serverAddress) override;
  bool start() override;
  void stop() override;
  void deinit() override;

  ActionCallStatus sendMessageToServer(const CSMessagePtr &msg) override;

  void onServerStatusChanged(Availability oldStatus,
                             Availability newStatus) noexcept override;

 protected:
  void monitorServerStatus(long long intervalMs);
  void onBytesCome(srz::Buffer &&bytes) override;

  Address myServerAddress_;

  std::thread serverMonitorThread_;
  std::thread receiverThread_;

  std::unique_ptr<BufferSenderIF> pSender_;
  std::unique_ptr<BufferReceiverIF> pReceiver_;

  std::promise<void> stopEventSource_;
};

}  // namespace local
}  // namespace ipc
}  // namespace messaging
}  // namespace maf
