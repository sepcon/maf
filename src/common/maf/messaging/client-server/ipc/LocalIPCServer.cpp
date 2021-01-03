#include "LocalIPCServer.h"

#include <maf/logging/Logger.h>
#include <maf/messaging/client-server/ServiceProviderIF.h>

#include <cassert>

#include "../SingleThreadPool.h"
#include "LocalIPCBufferReceiver.h"
#include "LocalIPCBufferSender.h"
#include "LocalIPCMessage.h"

namespace maf {
namespace messaging {
namespace ipc {
namespace local {

LocalIPCServer::LocalIPCServer()
    : pSender_{new LocalIPCBufferSender},
      pReceiver_{new LocalIPCBufferReceiver} {}

LocalIPCServer::~LocalIPCServer() = default;

bool LocalIPCServer::init(const Address &serverAddress) {
  if (pReceiver_->init(serverAddress)) {
    pReceiver_->setObserver(this);
    return true;
  }
  return false;
}

bool LocalIPCServer::start() {
  listeningThread_ = std::thread{[this] { pReceiver_->start(); }};
  return true;
}

void LocalIPCServer::stop() {
  if (pReceiver_->running()) {
    pReceiver_->stop();
  }
  if (listeningThread_.joinable()) {
    listeningThread_.join();
  }
}

void LocalIPCServer::deinit() {}

ActionCallStatus LocalIPCServer::sendMessageToClient(const CSMessagePtr &msg,
                                                     const Address &addr) {
  assert(msg != nullptr);
  if (pSender_) {
    try {
      return pSender_->send(
          std::static_pointer_cast<LocalIPCMessage>(msg)->toBytes(), addr);
    } catch (const std::bad_alloc &e) {
      MAF_LOGGER_ERROR("Message is too large to be serialized: ", e.what());
      return ActionCallStatus::FailedUnknown;
    }
  } else {
    MAF_LOGGER_ERROR(
        "Cannot send message due to null sender, please call init "
        "function before send function");
    return ActionCallStatus::ReceiverUnavailable;
  }
}

void LocalIPCServer::notifyServiceStatusToClient(const ServiceID &sid,
                                                 Availability oldStatus,
                                                 Availability newStatus) {
  if (oldStatus != newStatus) {
    auto serviceStatusMsg = createCSMessage<LocalIPCMessage>(
        sid,
        newStatus == Availability::Available ? OpID_ServiceAvailable
                                             : OpID_ServiceUnavailable,
        OpCode::ServiceStatusUpdate);

    std::lock_guard lock(registedClAddrs_);
    for (auto itAddr = registedClAddrs_->begin();
         itAddr != registedClAddrs_->end();) {
      auto ec = sendMessageToClient(serviceStatusMsg, *itAddr);
      if ((ec == ActionCallStatus::ReceiverUnavailable) ||
          (ec == ActionCallStatus::FailedUnknown)) {
        // Client has been off, then don't keep their contact anymore
        itAddr = registedClAddrs_->erase(itAddr);
      } else {
        ++itAddr;
      }
    }
  }
}

bool LocalIPCServer::onIncomingMessage(const CSMessagePtr &csMsg) {
  switch (csMsg->operationCode()) {
    case OpCode::RegisterServiceStatus:
      registedClAddrs_.atomic()->insert(csMsg->sourceAddress());
      {
        std::lock_guard lock(providers_);
        for (auto &[sid, provider] : *providers_) {
          if (provider->availability() == Availability::Available) {
            notifyServiceStatusToClient(csMsg->sourceAddress(), sid,
                                        Availability::Unavailable,
                                        Availability::Available);
          }
        }
      }
      return true;

    case OpCode::UnregisterServiceStatus:
      if (csMsg->serviceID() == ServiceIDInvalid) {
        registedClAddrs_.atomic()->erase(csMsg->sourceAddress());
        std::lock_guard lock(providers_);
        for (auto &[sid, provider] : *providers_) {
          csMsg->setServiceID(sid);
          provider->onIncomingMessage(csMsg);
        }
        return true;
      } else {
        break;
      }
    default:
      break;
  }

  return ServerBase::onIncomingMessage(csMsg);
}

void LocalIPCServer::onBytesCome(srz::Buffer &&buff) {
  single_threadpool::submit(
      [thisw = weak_from_this(), buff = std::move(buff)]() mutable {
        if (auto this_ = thisw.lock()) {
          std::shared_ptr<LocalIPCMessage> csMsg =
              std::make_shared<LocalIPCMessage>();
          if (csMsg->fromBytes(std::move(buff))) {
            std::static_pointer_cast<LocalIPCServer>(this_)->onIncomingMessage(
                csMsg);
          } else {
            MAF_LOGGER_ERROR("incoming message is not wellformed");
          }
        }
      });
}

void LocalIPCServer::notifyServiceStatusToClient(const Address &clAddr,
                                                 const ServiceID &sid,
                                                 Availability oldStatus,
                                                 Availability newStatus) {
  if (oldStatus != newStatus) {
    MAF_LOGGER_INFO("Update service ", sid,
                    " status to client at address: ", clAddr.dump());
    auto serviceStatusMsg = createCSMessage<LocalIPCMessage>(
        sid,
        newStatus == Availability::Available ? OpID_ServiceAvailable
                                             : OpID_ServiceUnavailable,
        OpCode::ServiceStatusUpdate);
    auto ec = sendMessageToClient(serviceStatusMsg, clAddr);
    if ((ec != ActionCallStatus::Success) &&
        (ec != ActionCallStatus::ReceiverBusy)) {
      // Don't need to remove client if failed?
    }
  }
}

}  // namespace local
}  // namespace ipc
}  // namespace messaging
}  // namespace maf
