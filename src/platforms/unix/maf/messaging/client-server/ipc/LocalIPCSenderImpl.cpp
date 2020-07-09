#include "LocalIPCSenderImpl.h"

#define ns_global

namespace maf {
namespace messaging {
namespace ipc {

namespace {

bool connectable(sockaddr_un *sockaddr) {
  AutoCloseFD<SockFD> fd;
  if (fd = socket(AF_UNIX, SOCK_STREAM, 0); fd == INVALID_FD) {
    MAF_SOCKET_ERROR("Cannot create socket");
  } else {
    if (connect(fd, _2sockAddr(sockaddr), sizeof(sockaddr_un)) == INVALID_FD) {
      // MAF_SOCKET_ERROR("Cannot connect to socket");
      fd.reset();
    }
  }
  return fd != INVALID_FD;
}
} // namespace

ActionCallStatus LocalIPCSenderImpl::send(const Buffer &payload,
                                          const Address &destination) {
  return send(payload, destination.get_name());
}

Availability
LocalIPCSenderImpl::checkReceiverStatus(const Address &destination) const {
  static thread_local sockaddr_un destSockAddr;
  static thread_local Address destAddr;
  if (destination != destAddr) {
    destAddr = destination;
    destSockAddr = createUnixAbstractSocketAddr(destAddr.get_name());
  }

  return connectable(&destSockAddr) ? Availability::Available
                                    : Availability::Unavailable;
}

ActionCallStatus LocalIPCSenderImpl::send(const Buffer &payload,
                                          const SocketPath &sockpath) {
  ActionCallStatus acs = ActionCallStatus::FailedUnknown;
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
          acs = ActionCallStatus::FailedUnknown;
          break;
        }
      } while (totalWritten < payloadSize);

      acs = ActionCallStatus::Success;
    } else {
      MAF_SOCKET_ERROR("Failed to send payload size[", payloadSize,
                       "] to receiver");
    }

    if (acs != ActionCallStatus::Success) {
      if (totalWritten == 0) {

      } else {
        // ec = FailedUnknown, must provide more info for debugging purpose
        MAF_LOGGER_ERROR("Failed to send payload to receiver, expected is ",
                         sizeof(SizeType), ", sent was ", totalWritten);
      }
    }
  } else {
    acs = ActionCallStatus::ReceiverUnavailable;
  }
  return acs;
}

} // namespace ipc
} // namespace messaging
} // namespace maf
