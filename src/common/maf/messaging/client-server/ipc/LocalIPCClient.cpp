#include "LocalIPCClient.h"

#include <maf/logging/Logger.h>
#include <maf/utils/Process.h>

#include "../GlobalThreadPool.h"
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
  global_threadpool::tryAddThread();
  global_threadpool::submit([this] { monitorServerStatus(); });
  return global_threadpool::threadCount() > 0;
}

void LocalIPCClient::stop() {
  pReceiver_->stop();
  serverMonitorTimer_.stop();
  if (receiverThread_.joinable()) {
    receiverThread_.join();
  }
  global_threadpool::tryRemoveThread();
}

void LocalIPCClient::deinit() { ClientBase::deinit(); }

LocalIPCClient::~LocalIPCClient() { deinit(); }

ActionCallStatus LocalIPCClient::sendMessageToServer(const CSMessagePtr &msg) {
  assert(msg != nullptr);
  try {
    msg->setSourceAddress(pReceiver_->address());
    return pSender_->send(
        std::static_pointer_cast<LocalIPCMessage>(msg)->toBytes(),
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
    auto registeredMsg = messaging::createCSMessage<LocalIPCMessage>(
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

void LocalIPCClient::monitorServerStatus(long long tunedInterval) {
  if (auto newStatus = pSender_->checkReceiverStatus(myServerAddress_);
      currentServerStatus_ != newStatus) {
    tunedInterval = serverMonitorInterval;
    this->onServerStatusChanged(currentServerStatus_, newStatus);
    currentServerStatus_ = newStatus;

  } else if (tunedInterval < serverMonitorInterval) {
    tunedInterval += 5;
  }
  serverMonitorTimer_.start(tunedInterval,
                            [=] { this->monitorServerStatus(tunedInterval); });
}

void LocalIPCClient::onBytesCome(srz::Buffer &&buff) {
  global_threadpool::submit([this, buff = std::move(buff)]() mutable {
    std::shared_ptr<LocalIPCMessage> csMsg =
        std::make_shared<LocalIPCMessage>();
    if (csMsg->fromBytes(std::move(buff))) {
      onIncomingMessage(csMsg);
    } else {
      MAF_LOGGER_ERROR("incoming message is not wellformed");
    }
  });
}

}  // namespace local
}  // namespace ipc
}  // namespace messaging
}  // namespace maf
