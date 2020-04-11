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
    monitorServerStatus(sersverMonitoringIntervalMs);
    return true;
  } else {
    return false;
  }
}

bool IPCClientBase::deinit() {
  {
    std::lock_guard lock(mutex_);
    if (!stopped_)
      stopped_ = true;
    stopCondition_.notify_one();
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
  msg->setSourceAddress(pReceiver_->address());
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
  serverMonitorThread_ = std::thread([this, intervalMs] {
    std::unique_lock lock(mutex_);
    auto oldStatus = Availability::Unavailable;
    auto tunnedInterval = decltype(intervalMs){0};
    do {
      if (auto newStatus = pSender_->checkReceiverStatus();
          oldStatus != newStatus) {
        tunnedInterval = intervalMs;
        this->onServerStatusChanged(oldStatus, newStatus);
        oldStatus = newStatus;
      } else if (newStatus == Availability::Unavailable) {
        if (tunnedInterval < intervalMs) {
          tunnedInterval += 10;
        }
      }
      stopCondition_.wait_for(lock, std::chrono::milliseconds{tunnedInterval});
    } while (!stopped_);
  });
}

} // namespace ipc
} // namespace messaging
} // namespace maf
