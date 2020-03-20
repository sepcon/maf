#include "IPCServerBase.h"
#include "BytesCommunicator.h"
#include <maf/logging/Logger.h>
#include <maf/messaging/client-server/ServiceProviderIF.h>

namespace maf {
namespace messaging {
namespace ipc {

IPCServerBase::IPCServerBase(IPCType ipctype)
    : communicator_(
          new BytesCommunicator(ipctype, this, /*isclient = */ false)) {}

IPCServerBase::~IPCServerBase() { delete communicator_; }

bool IPCServerBase::init(const Address &serverAddress) {
  return communicator_->init(serverAddress);
}

bool IPCServerBase::deinit() {
  return communicator_->deinit() && ServerBase::deinit();
}

ActionCallStatus IPCServerBase::sendMessageToClient(const CSMessagePtr &msg,
                                                    const Address &addr) {
  return communicator_->send(std::static_pointer_cast<IPCMessage>(msg), addr);
}

void IPCServerBase::notifyServiceStatusToClient(const ServiceID &sid,
                                                Availability oldStatus,
                                                Availability newStatus) {
  if (oldStatus != newStatus) {
    auto serviceStatusMsg = createCSMessage<IPCMessage>(
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

bool IPCServerBase::onIncomingMessage(const CSMessagePtr &csMsg) {
  switch (csMsg->operationCode()) {
  case OpCode::RegisterServiceStatus:
    registedClAddrs_.atomic()->insert(csMsg->sourceAddress());
    {
      std::lock_guard lock(providers_);
      for (auto &[sid, provider] : *providers_) {
        notifyServiceStatusToClient(
            csMsg->sourceAddress(), provider->serviceID(),
            Availability::Unavailable, Availability::Available);
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

void IPCServerBase::notifyServiceStatusToClient(const Address &clAddr,
                                                const ServiceID &sid,
                                                Availability oldStatus,
                                                Availability newStatus) {
  if (oldStatus != newStatus) {
    MAF_LOGGER_INFO("Update service ", sid,
                    " status to client at address: ", clAddr.dump());
    auto serviceStatusMsg = createCSMessage<IPCMessage>(
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

} // namespace ipc
} // namespace messaging
} // namespace maf
