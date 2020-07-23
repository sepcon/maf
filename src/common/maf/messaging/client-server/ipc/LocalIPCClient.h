#pragma once

#include <maf/messaging/Timer.h>

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
  void monitorServerStatus(long long intervalMs = 0);
  void onBytesCome(srz::Buffer &&buff) override;

  Address myServerAddress_;

  Timer serverMonitorTimer_;
  std::thread receiverThread_;

  std::unique_ptr<BufferSenderIF> pSender_;
  std::unique_ptr<BufferReceiverIF> pReceiver_;

  Availability currentServerStatus_ = Availability::Unavailable;
  int serverMonitorInterval = 500;
};

}  // namespace local
}  // namespace ipc
}  // namespace messaging
}  // namespace maf
