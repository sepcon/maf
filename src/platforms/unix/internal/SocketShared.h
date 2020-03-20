#ifndef SOCKETSHARED_H
#define SOCKETSHARED_H
#include <error.h>
#include <maf/logging/Logger.h>
#include <maf/messaging/client-server/Address.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

namespace maf {
namespace messaging {
namespace ipc {

using SizeType = uint32_t;
using SocketPath = std::string;
using FD = int;
using SockFD = FD;

static constexpr size_t MAXCLIENTS = 30;
static constexpr SockFD INVALID_FD = -1;
template <typename FileDescriptor, FD INVALID_VALUE = INVALID_FD>
class AutoCloseFD {
  static_assert(std::is_same_v<std::decay_t<FileDescriptor>, FD>,
                "FileDescriptor must be a type of FD `const FD&` or `FD` ");

public:
  AutoCloseFD(FileDescriptor fd_ = INVALID_VALUE) : fd{fd_} {}
  AutoCloseFD(AutoCloseFD &&rhs) { takefrom(std::move(rhs)); }
  AutoCloseFD &operator=(AutoCloseFD &&rhs) {
    takefrom(std::move(rhs));
    return *this;
  }
  ~AutoCloseFD() { closeFD(); }
  void reset() { closeFD(); }
  operator FileDescriptor() { return fd; }

private:
  void takefrom(AutoCloseFD &&rhs) {
    closeFD();
    fd = rhs.fd;
    rhs.fd = INVALID_VALUE;
  }
  void closeFD() {
    if (fd != INVALID_VALUE) {
      close(fd);
      fd = INVALID_VALUE;
    }
  }
  FileDescriptor fd;
};

#define MAF_SOCKET_ERROR(...)                                                  \
  MAF_LOGGER_ERROR(__VA_ARGS__, " with errno = ", errno, "!");

inline bool isValidSocketPath(const SocketPath &path) {
  return sizeof(sockaddr_un::sun_path) > path.length();
}

inline SocketPath constructSocketPath(const Address &addr) {
  return addr.get_name() + "/" + std::to_string(addr.get_port());
}

inline sockaddr_un createUnixAbstractSocketAddr(const SocketPath &sockpath) {
  sockaddr_un addr;
  memset(&addr, 0, sizeof(addr));
  addr.sun_family = AF_UNIX;
  strncpy(addr.sun_path + 1, sockpath.data(), sockpath.length());
  return addr;
}

template <typename SockAddress> inline sockaddr *_2sockAddr(SockAddress *sa) {
  return reinterpret_cast<sockaddr *>(sa);
}

inline AutoCloseFD<SockFD> connectToSocket(const std::string &sockpath) {
  AutoCloseFD<SockFD> fd;
  if (isValidSocketPath(sockpath)) {
    if (fd = socket(AF_UNIX, SOCK_STREAM, 0); fd == INVALID_FD) {
      MAF_SOCKET_ERROR("Could not allocate new socket");
    } else {
      auto addr = createUnixAbstractSocketAddr(sockpath);
      if (connect(fd, _2sockAddr(&addr), sizeof(addr)) == INVALID_FD) {
        MAF_SOCKET_ERROR("Can't connect to address ", sockpath);
        fd.reset();
      }
    }
  } else {
    MAF_LOGGER_ERROR(
        "Length of address exeeds the limitation of unix domain socket path");
  }

  return fd;
}
} // namespace ipc
} // namespace messaging
} // namespace maf
#endif // SOCKETSHARED_H
