#include "LocalIPCBufferReceiverImpl.h"

#include <arpa/inet.h>
#include <maf/logging/Logger.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "SocketShared.h"

namespace maf {
namespace messaging {
namespace ipc {
namespace local {

LocalIPCBufferReceiverImpl::~LocalIPCBufferReceiverImpl() { stop(); }

bool LocalIPCBufferReceiverImpl::init(const Address &addr) {
  bool startable = false;
  myaddr_ = addr;
  auto sockpath = myaddr_.get_name();
  if (isValidSocketPath(sockpath)) {
    mySockAddr_ = createUnixAbstractSocketAddr(sockpath);
    setState(State::Initialized);
    // Create the socket.
    int opt = true;
    fdMySock_ = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fdMySock_ != INVALID_FD &&
        (setsockopt(fdMySock_, SOL_SOCKET, SO_REUSEADDR,
                    reinterpret_cast<char *>(&opt), sizeof(opt)) >= 0)) {
      if (bind(fdMySock_, _2sockAddr(&mySockAddr_), sizeof(mySockAddr_)) >= 0) {
        // Max 5 pending connections
        if (listen(fdMySock_, 5) == 0) {
          MAF_LOGGER_INFO("Listening on address ", myaddr_.dump());
          startable = true;
        } else {
          MAF_LOGGER_ERROR("Could not listen on socket");
        }
      } else {
        MAF_LOGGER_ERROR("Coud not bind socket to address ", myaddr_.dump());
      }
    }
  }

  return startable;
}

bool LocalIPCBufferReceiverImpl::start() {
  try {
    waitAndProcessConnections();
  } catch (StoppedInterruption) {
  } catch (...) {
    return false;
  }
  return true;
}

void LocalIPCBufferReceiverImpl::stop() {
  if (running()) {
    auto currentState = getState();
    setState(State::Stopped);
    // trying connect to socket to wake the running thread
    if (currentState == State::WaitingConnection) {
      connectToSocket(myaddr_.get_name());
    }
  }
}

void LocalIPCBufferReceiverImpl::deinit() {}

bool LocalIPCBufferReceiverImpl::running() const {
  switch (getState()) {
    case State::Running:
    case State::WaitingConnection:
      return true;
    default:
      return false;
  }
}

const Address &LocalIPCBufferReceiverImpl::address() const { return myaddr_; }

void LocalIPCBufferReceiverImpl::setObserver(BytesComeCallback callback) {
  bytesComeCallback_ = std::move(callback);
}

bool LocalIPCBufferReceiverImpl::waitAndProcessConnections() {
  auto maxSd = INVALID_FD;
  socklen_t sockLen = sizeof(mySockAddr_);
  std::vector<int> fdClientSocks(MAXCLIENTS, 0);
  fd_set readfds;
  do {
    // clear the socket set
    FD_ZERO(&readfds);

    // add master socket to set
    FD_SET(fdMySock_, &readfds);
    maxSd = fdMySock_;

    // add child sockets to set
    for (size_t i = 0; i < MAXCLIENTS; i++) {
      // socket descriptor
      auto sd = fdClientSocks[i];

      // if valid socket descriptor then add to read list
      if (sd > 0) FD_SET(sd, &readfds);

      // highest file descriptor number, need it for the select function
      if (sd > maxSd) maxSd = sd;
    }

    timeval timeout = {1, 0};

    interruptionPoint();

    // wait for an activity on one of the sockets
    setState(State::WaitingConnection);

    auto totalSD = select(maxSd + 1, &readfds, nullptr, nullptr, &timeout);

    interruptionPoint();

    setState(State::Running);

    if (totalSD <= 0) {
      continue;
    }

    // If something happened on the master socket ,
    // then its an incoming connection
    if (FD_ISSET(fdMySock_, &readfds)) {
      auto acceptedSD = accept(fdMySock_, _2sockAddr(&mySockAddr_), &sockLen);
      if (acceptedSD < 0) {
        MAF_LOGGER_ERROR("Failed on accepting new socket connection");
        return false;
      }

      // add new socket to array of sockets
      for (size_t i = 0; i < MAXCLIENTS; i++) {
        // if position is empty
        if (fdClientSocks[i] == 0) {
          fdClientSocks[i] = acceptedSD;
          break;
        }
      }
    }

    // else its some IO operation on some other socket
    for (size_t i = 0; i < MAXCLIENTS; i++) {
      auto sd = fdClientSocks[i];
      if (FD_ISSET(sd, &readfds)) {
        SizeType messageLength = 0;
        bool failed = true;
        ssize_t totalRead = 0;
        if (auto bytesRead = read(sd, reinterpret_cast<char *>(&messageLength),
                                  sizeof(SizeType));
            bytesRead == sizeof(SizeType)) {
          auto payload = srz::Buffer{};
          payload.resize(messageLength);
          while (totalRead < messageLength) {
            if (bytesRead = read(sd, payload.data() + totalRead, messageLength);
                bytesRead != -1) {
              totalRead += bytesRead;
            } else {
              MAF_SOCKET_ERROR("Could not read bytes from socket total read = ",
                               totalRead, " total expected = ", messageLength);
              break;
            }
          }
          if (totalRead == messageLength) {
            bytesComeCallback_(std::move(payload));
            failed = false;
          }
        }
        close(sd);
        fdClientSocks[i] = 0;
      }
    }
  } while (true);

  setState(State::Stopped);
  return true;
}

void LocalIPCBufferReceiverImpl::interruptionPoint() {
  if (getState() == State::Stopped) {
    MAF_LOGGER_INFO("Finish running due to flag STOP was turned on, address: ",
                    myaddr_.dump());
    throw StoppedInterruption{};
  }
}

}  // namespace local
}  // namespace ipc
}  // namespace messaging
}  // namespace maf
