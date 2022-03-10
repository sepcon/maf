#include "LocalIPCClient.h"

#include <maf/logging/Logger.h>
#include <maf/messaging/Timer.h>
#include <maf/utils/Process.h>

#include <cassert>
#include <future>
#include <thread>

#include "../ClientBase.h"
#include "../SingleThreadPool.h"
#include "BufferReceiverIF.h"
#include "BufferSenderIF.h"
#include "IPCTypes.h"
#include "LocalIPCBufferReceiver.h"
#include "LocalIPCBufferSender.h"
#include "LocalIPCMessage.h"

namespace maf {
namespace messaging {
namespace ipc {
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
  single_threadpool::submit([this] { monitorServerStatus(); });
  return true;
}

void LocalIPCClient::stop() {
  pReceiver_->stop();
  serverMonitorTimer_.stop();
  if (receiverThread_.joinable()) {
    receiverThread_.join();
  }
}

void LocalIPCClient::deinit() { ClientBase::deinit(); }

LocalIPCClient::~LocalIPCClient() { LocalIPCClient::deinit(); }

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
    if (auto callStatus = sendMessageToServer(registeredMsg);
        callStatus == ActionCallStatus::Success) {
      MAF_LOGGER_INFO(
          "Send service status change register to server successfully! from ",
          pReceiver_->address().dump());
      currentServerStatus_ = newStatus;
    } else {
      MAF_LOGGER_ERROR(
          "Could not send service status register request to server: ",
          callStatus);
    }
  } else {
    currentServerStatus_ = newStatus;
  }
}

void LocalIPCClient::monitorServerStatus(long long tunedInterval) {
  if (auto newStatus = pSender_->checkReceiverStatus(myServerAddress_);
      currentServerStatus_ != newStatus) {
    MAF_LOGGER_INFO("Server (", myServerAddress_.get_name(),
                    ")'s status changed from: ", currentServerStatus_, " to ",
                    newStatus);
    tunedInterval = serverMonitorInterval;
    this->onServerStatusChanged(currentServerStatus_, newStatus);
  } else if (tunedInterval < serverMonitorInterval) {
    tunedInterval += 5;
  }
  serverMonitorTimer_.start(tunedInterval, [tunedInterval, this] {
    this->monitorServerStatus(tunedInterval);
  });
}

void LocalIPCClient::onBytesCome(srz::Buffer &&buff) {
  single_threadpool::submit([this, buff = std::move(buff)]() mutable {
    std::shared_ptr<LocalIPCMessage> csMsg =
        std::make_shared<LocalIPCMessage>();
    if (csMsg->fromBytes(std::move(buff))) {
      onIncomingMessage(csMsg);
    } else {
      MAF_LOGGER_ERROR("incoming message is not wellformed");
    }
  });
}

std::shared_ptr<ClientIF> makeClient() {
  return std::make_shared<LocalIPCClient>();
}

}  // namespace local
}  // namespace ipc
}  // namespace messaging
}  // namespace maf
