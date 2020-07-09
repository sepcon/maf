#pragma once

#include "../ClientBase.h"
#include "IPCReceiverIF.h"
#include "IPCTypes.h"

#include <future>
#include <thread>

namespace maf {
namespace messaging {
namespace ipc {

class IPCSenderIF;
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
  void onBytesCome(srz::ByteArray &&bytes) override;

  Address myServerAddress_;

  std::thread serverMonitorThread_;
  std::thread receiverThread_;

  std::unique_ptr<IPCSenderIF> pSender_;
  std::unique_ptr<IPCReceiverIF> pReceiver_;

  std::promise<void> stopEventSource_;
};

} // namespace ipc
} // namespace messaging
} // namespace maf
