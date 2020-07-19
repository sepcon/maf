#include "LocalIPCClient.h"

#include <maf/logging/Logger.h>
#include <maf/utils/Process.h>

#include "BufferSenderIF.h"
#include "LocalIPCBufferReceiver.h"
#include "LocalIPCBufferSender.h"
#include "LocalIPCMessage.h"

namespace maf {
namespace messaging {
namespace ipc {
namespace local {

LocalIPCClient::LocalIPCClient()
    : pSender_{new local::LocalIPCBufferSender},
      pReceiver_{new LocalIPCBufferReceiver} {}

bool LocalIPCClient::init(const Address &serverAddress) {
  assert(serverAddress.valid());
  auto myReceiverAddress =
      Address(serverAddress.get_name() + std::to_string(util::process::pid()),
              serverAddress.get_port());

  if (pReceiver_->init(myReceiverAddress)) {
    myServerAddress_ = serverAddress;
    pReceiver_->setObserver(this);
    return true;
  }
  return false;
}

bool LocalIPCClient::start() {
  receiverThread_ = std::thread{[this] { pReceiver_->start(); }};
  monitorServerStatus(500);
  return true;
}

void LocalIPCClient::stop() {
  pReceiver_->stop();
  stopEventSource_.set_value();
  if (serverMonitorThread_.joinable()) {
    serverMonitorThread_.join();
  }
  if (receiverThread_.joinable()) {
    receiverThread_.join();
  }
}

void LocalIPCClient::deinit() { ClientBase::deinit(); }

LocalIPCClient::~LocalIPCClient() { deinit(); }

ActionCallStatus LocalIPCClient::sendMessageToServer(const CSMessagePtr &msg) {
  assert(msg != nullptr);
  try {
    msg->setSourceAddress(pReceiver_->address());
    return pSender_->send(
        std::static_pointer_cast<local::LocalIPCMessage>(msg)->toBytes(),
        myServerAddress_);
  } catch (const std::bad_alloc &e) {
    MAF_LOGGER_ERROR("Message is too large to be serialized: ", e.what());
    return ActionCallStatus::FailedUnknown;
  }
}

void LocalIPCClient::onServerStatusChanged(Availability oldStatus,
                                           Availability newStatus) noexcept {
  ClientBase::onServerStatusChanged(oldStatus, newStatus);
  if (newStatus == Availability::Available) {
    auto registeredMsg = messaging::createCSMessage<local::LocalIPCMessage>(
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

void LocalIPCClient::monitorServerStatus(long long intervalMs) {
  serverMonitorThread_ = std::thread(
      [this, stopEvent = stopEventSource_.get_future(), intervalMs] {
        auto oldStatus = Availability::Unavailable;
        auto tunnedInterval = decltype(intervalMs){0};
        do {
          if (auto newStatus = pSender_->checkReceiverStatus(myServerAddress_);
              oldStatus != newStatus) {
            tunnedInterval = intervalMs;
            this->onServerStatusChanged(oldStatus, newStatus);
            oldStatus = newStatus;
          } else if (newStatus == Availability::Unavailable) {
            if (tunnedInterval < intervalMs) {
              tunnedInterval += 10;
            }
          }
          if (stopEvent.wait_for(std::chrono::milliseconds{tunnedInterval}) ==
              std::future_status::ready) {
            break;
          }
        } while (true);
      });
}

void LocalIPCClient::onBytesCome(srz::Buffer &&bytes) {
  std::shared_ptr<local::LocalIPCMessage> csMsg =
      std::make_shared<local::LocalIPCMessage>();
  if (csMsg->fromBytes(std::move(bytes))) {
    onIncomingMessage(csMsg);
  } else {
    MAF_LOGGER_ERROR("incoming message is not wellformed");
  }
}

}  // namespace local
}  // namespace ipc
}  // namespace messaging
}  // namespace maf
