#include "LocalIPCSenderImpl.h"

#define ns_global

namespace maf {
namespace messaging {
namespace ipc {

namespace {

bool connectable(const sockaddr_un &sockaddr) {
  AutoCloseFD<SockFD> fd;
  if (fd = socket(AF_UNIX, SOCK_STREAM, 0); fd == INVALID_FD) {
    MAF_SOCKET_ERROR("Cannot create socket");
  } else {
    if (connect(fd, _2sockAddr(const_cast<sockaddr_un *>(&sockaddr)),
                sizeof(sockaddr)) == INVALID_FD) {
      fd.reset();
    }
  }
  return fd != INVALID_FD;
}
} // namespace

LocalIPCSenderImpl::LocalIPCSenderImpl() {}

LocalIPCSenderImpl::~LocalIPCSenderImpl() {}

ActionCallStatus LocalIPCSenderImpl::send(const srz::ByteArray &payload,
                                          const Address &destination) {
  ActionCallStatus ec = ActionCallStatus::FailedUnknown;
  SocketPath sockpath;

  if (destination.valid()) {
    sockpath = constructSocketPath(destination);
  } else if (_myReceiverAddr.valid()) {
    sockpath = *_myReceiverSocketPath;
  }

  if (auto fd = connectToSocket(sockpath); fd != INVALID_FD) {
    SizeType totalWritten = 0;
    SizeType payloadSize = static_cast<SizeType>(payload.length());

    // First write is to send payload size to receiver to reserve a buffer
    if (auto written = ns_global::write(
            fd, reinterpret_cast<char *>(&payloadSize), sizeof(SizeType));
        written == sizeof(SizeType)) {
      // Second write is to send payload content
      do {
        if (written = ns_global::write(fd, payload.data() + totalWritten,
                                       payloadSize - totalWritten);
            written != -1) {
          totalWritten += written;
        } else {
          MAF_SOCKET_ERROR("Failed to send bytes to receiver, total written = ",
                           totalWritten);
          ec = ActionCallStatus::FailedUnknown;
          break;
        }
      } while (totalWritten < payloadSize);

      ec = ActionCallStatus::Success;
    } else {
      MAF_SOCKET_ERROR("Failed to send payload size[", payloadSize,
                       "] to receiver");
    }

    if (ec != ActionCallStatus::Success) {
      if (totalWritten == 0) {

      } else {
        // ec = FailedUnknown, must provide more info for debugging purpose
        MAF_LOGGER_ERROR("Failed to send payload to receiver, expected is ",
                         sizeof(SizeType), ", sent was ", totalWritten);
      }
    }
  } else {
    ec = ActionCallStatus::ReceiverUnavailable;
  }

  return ec;
}

bool LocalIPCSenderImpl::initConnection(const Address &receiverAddr) {
  _myReceiverAddr = receiverAddr;
  auto socketPath = constructSocketPath(receiverAddr);
  if (isValidSocketPath(socketPath)) {
    _myReceiverSocketPath = std::make_unique<SocketPath>(std::move(socketPath));
    _myReceiverSockAddr = std::make_unique<sockaddr_un>(
        createUnixAbstractSocketAddr(*_myReceiverSocketPath));
    return true;
  }
  return false;
}

Availability LocalIPCSenderImpl::checkReceiverStatus() const {
  auto status = Availability::Unknown;
  if (_myReceiverSocketPath && _myReceiverSockAddr) {
    status = connectable(*_myReceiverSockAddr) ? Availability::Available
                                               : Availability::Unavailable;
  }
  return status;
}

const Address &LocalIPCSenderImpl::receiverAddress() const {
  return _myReceiverAddr;
}

} // namespace ipc
} // namespace messaging
} // namespace maf
