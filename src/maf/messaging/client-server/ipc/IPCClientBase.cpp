#include "IPCClientBase.h"
#include "BytesCommunicator.h"
#include <maf/logging/Logger.h>
#include <maf/messaging/SyncTimer.h>

namespace maf {

namespace messaging {
namespace ipc {

IPCClientBase::IPCClientBase(IPCType type)
    : BytesCommunicator(type, this, /*isclient = */ true) {}

bool IPCClientBase::init(const Address &serverAddress,
                         long long sersverMonitoringIntervalMs) {
  if (BytesCommunicator::init(serverAddress)) {
    serverMonitorTimer_ = std::make_unique<SyncTimer>(true);
    monitorServerStatus(sersverMonitoringIntervalMs);
    return true;
  } else {
    return false;
  }
}

bool IPCClientBase::deinit() {
  if (serverMonitorTimer_) {
    serverMonitorTimer_->stop();
  }
  ClientBase::deinit();
  BytesCommunicator::deinit();
  if (serverMonitorThread_.joinable()) {
    serverMonitorThread_.join();
  }
  return true;
}

IPCClientBase::~IPCClientBase() { deinit(); }

ActionCallStatus IPCClientBase::sendMessageToServer(const CSMessagePtr &msg) {
  msg->setSourceAddress(_pReceiver->address());
  return BytesCommunicator::send(std::static_pointer_cast<IPCMessage>(msg));
}

void IPCClientBase::onServerStatusChanged(Availability oldStatus,
                                          Availability newStatus) noexcept {
  ClientBase::onServerStatusChanged(oldStatus, newStatus);
  if (newStatus == Availability::Available) {
    auto registeredMsg = messaging::createCSMessage<IPCMessage>(
        ServiceIDInvalid, OpIDInvalid, OpCode::RegisterServiceStatus);
    if (sendMessageToServer(registeredMsg) == ActionCallStatus::Success) {
      MAF_LOGGER_INFO(
          "Send service status change register to server successfully!");
    } else {
      MAF_LOGGER_INFO(
          "Could not send service status register request to server");
    }
  }
}

void IPCClientBase::monitorServerStatus(long long intervalMs) {
  serverMonitorThread_ = std::thread{[this, intervalMs] {
    auto oldStatus = Availability::Unavailable;
    auto checkAndNotifyServerStatus = [this, &oldStatus] {
      auto newStatus = _pSender->checkReceiverStatus();
      if (oldStatus != newStatus) {
        onServerStatusChanged(oldStatus, newStatus);
        oldStatus = newStatus;
      }
    };

    // Check once at the begining
    checkAndNotifyServerStatus();
    serverMonitorTimer_->start(intervalMs,
                               std::move(checkAndNotifyServerStatus));
  }};
}

} // namespace ipc
} // namespace messaging
} // namespace maf
