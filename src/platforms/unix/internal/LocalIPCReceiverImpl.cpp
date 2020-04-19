#include "LocalIPCReceiverImpl.h"
#include "SocketShared.h"
#include <maf/logging/Logger.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <sys/stat.h>

namespace maf {
namespace messaging {
namespace ipc {

#define ns_global

LocalIPCReceiverImpl::~LocalIPCReceiverImpl() { stopListening(); }

bool LocalIPCReceiverImpl::initConnection(const Address &addr,
                                          bool isClientMode) {
  stopped_ = true;
  if (isClientMode) {
    static std::atomic<uint16_t> receiverCount(0);
    receiverCount += 1;
    uint16_t randomPort = receiverCount;
    myaddr_ = Address(addr.get_name() + std::to_string(getpid()), randomPort);
  } else {
    myaddr_ = std::move(addr);
  }
  auto sockpath = constructSocketPath(myaddr_);
  if (isValidSocketPath(sockpath)) {
    mySockAddr_ = createUnixAbstractSocketAddr(sockpath);
    return true;
  } else {
    return false;
  }
}

bool LocalIPCReceiverImpl::startListening() {
  bool startable = false;

  // Create the socket.
  int opt = true;
  auto fdMySock = socket(AF_UNIX, SOCK_STREAM, 0);
  if (fdMySock != INVALID_FD &&
      (setsockopt(fdMySock, SOL_SOCKET, SO_REUSEADDR,
                  reinterpret_cast<char *>(&opt), sizeof(opt)) >= 0)) {
    if (bind(fdMySock, _2sockAddr(&mySockAddr_), sizeof(mySockAddr_)) >= 0) {
      // Max 5 pending connections
      if (listen(fdMySock, 5) == 0) {
        MAF_LOGGER_INFO("Listening on address ", myaddr_.dump());
        stopped_ = false;
        listeningThread_ = std::thread{
            &LocalIPCReceiverImpl::listeningThreadFunc, this, fdMySock};
        startable = true;
      } else {
        MAF_LOGGER_ERROR("Could not listen on socket");
      }
    } else {
      MAF_LOGGER_ERROR("Coud not bind socket to address ", myaddr_.dump());
    }
  }

  return startable;
}

bool LocalIPCReceiverImpl::stopListening() {
  bool ret = false;

  if (listening()) {
    stopped_.store(true, std::memory_order_release);
    // trying connect to socket to wake the listening thread
    if (selecting_.load(std::memory_order_acquire)) {
      connectToSocket(constructSocketPath(myaddr_));
    }

    ret = true;
  }

  if (listeningThread_.joinable()) {
    listeningThread_.join();
  }

  return ret;
}

bool LocalIPCReceiverImpl::listening() const {
  return !stopped_.load(std::memory_order_acquire);
}

const Address &LocalIPCReceiverImpl::address() const { return myaddr_; }

void LocalIPCReceiverImpl::registerObserver(BytesComeCallback callback) {
  bytesComeCallback_ = std::move(callback);
}

bool LocalIPCReceiverImpl::listeningThreadFunc(int fdMySock) {
  auto maxSd = INVALID_FD;
  socklen_t sockLen = sizeof(mySockAddr_);
  std::vector<int> fdClientSocks(MAXCLIENTS, 0);
  fd_set readfds;
  while (true) {
    // clear the socket set
    FD_ZERO(&readfds);

    // add master socket to set
    FD_SET(fdMySock, &readfds);
    maxSd = fdMySock;

    // add child sockets to set
    for (size_t i = 0; i < MAXCLIENTS; i++) {
      // socket descriptor
      auto sd = fdClientSocks[i];

      // if valid socket descriptor then add to read list
      if (sd > 0)
        FD_SET(sd, &readfds);

      // highest file descriptor number, need it for the select function
      if (sd > maxSd)
        maxSd = sd;
    }

    timeval timeout = {1, 0};
    // wait for an activity on one of the sockets
    selecting_.store(true, std::memory_order_release);

    auto totalSD = select(maxSd + 1, &readfds, nullptr, nullptr, &timeout);

    selecting_.store(false, std::memory_order_release);

    if (stopped_.load(std::memory_order_acquire)) {
      MAF_LOGGER_INFO(
          "Finish listening due to flag STOP was turned on, address: ",
          myaddr_.dump());
      break;
    }

    if (totalSD <= 0) {
      continue;
    }

    // If something happened on the master socket ,
    // then its an incoming connection
    if (FD_ISSET(fdMySock, &readfds)) {
      auto acceptedSD = accept(fdMySock, _2sockAddr(&mySockAddr_), &sockLen);
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
          auto payload = srz::ByteArray{};
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
  }

  stopped_ = true;
  return true;
}

} // namespace ipc
} // namespace messaging
} // namespace maf
