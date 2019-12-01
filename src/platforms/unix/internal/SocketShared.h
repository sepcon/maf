#ifndef SOCKETSHARED_H
#define SOCKETSHARED_H
#include <maf/logging/Logger.h>
#include <maf/messaging/client-server/Address.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <unistd.h>
#include <error.h>

namespace maf {
namespace messaging {
namespace ipc {

using SizeType = uint32_t;
using SocketPath = std::string;
using FD = int;
using SockFD = FD;

static constexpr size_t MAXCLIENTS = 30;
static constexpr SockFD INVALID_FD = -1;


template <typename... Msgs>
void socketError(Msgs&&... msgs)
{
    logging::Logger::error(std::forward<Msgs>(msgs)..., " with errno = ", errno, "!");
}

inline bool isValidSocketPath(const SocketPath& path)
{
    return sizeof(sockaddr_un::sun_path) > path.length();
}

inline SocketPath constructSocketPath(const Address& addr)
{
    return addr.get_name() + "/" + std::to_string(addr.get_port());
}

inline sockaddr_un createUnixAbstractSocketAddr(const SocketPath& sockpath)
{
    sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path + 1, sockpath.data(), sockpath.length());
    return addr;
}

template <typename SockAddress>
inline sockaddr* _2sockAddr(SockAddress* sa)
{
    return reinterpret_cast<sockaddr*>(sa);
}

template<typename FileDescriptor, FD INVALID_VALUE = INVALID_FD>
class AutoCloseFD
{
    static_assert (std::is_same_v<std::decay_t<FileDescriptor>, FD>, "FileDescriptor must be a type of FD `const FD&` or `FD` ");
public:
    AutoCloseFD(FileDescriptor fd_ = INVALID_VALUE) : fd{fd_}{}
    AutoCloseFD(AutoCloseFD&& rhs) { takefrom(std::move(rhs)); }
    AutoCloseFD& operator=(AutoCloseFD&& rhs) { takefrom(std::move(rhs)); return *this; }
    ~AutoCloseFD() { closeFD(); }
    void reset() { closeFD(); }
    operator FileDescriptor() { return fd; }
private:
    void takefrom(AutoCloseFD&& rhs)
    {
        closeFD(); fd = rhs.fd; rhs.fd = INVALID_VALUE;
    }
    void closeFD()
    {
        if(fd != INVALID_VALUE )
        {
            close(fd);
            fd = INVALID_VALUE;
        }
    }
    FileDescriptor fd;
};
}
}
}
#endif // SOCKETSHARED_H
