#include <maf/utils/debugging/Debug.h>
#include "SocketShared.h"
#include "LocalIPCReceiverImpl.h"

#include <stdlib.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
//#include <fcntl.h> // for open
#include <pthread.h>

namespace maf {
namespace messaging {
namespace ipc {

#define ns_global

LocalIPCReceiverImpl::~LocalIPCReceiverImpl()
{
    stopListening();
}

bool LocalIPCReceiverImpl::initConnection(const Address &addr, bool isClientMode)
{
    _stopped = true;
    _isClient = isClientMode;
    if(isClientMode)
    {
        static std::atomic<uint16_t> receiverCount(0);
        receiverCount += 1;
        uint16_t randomPort = receiverCount;
        _myaddr = Address(addr.name() + std::to_string(getpid()), randomPort);
    }
    else
    {
        _myaddr = std::move(addr);
    }
    auto sockpath = constructSocketPath(_myaddr);
    if(isValidSocketPath(sockpath))
    {
        _mySockAddr = createUnixAbstractSocketAddr(sockpath);
        return true;
    }
    else
    {
        return false;
    }
}

bool LocalIPCReceiverImpl::startListening()
{
    bool startable = false;

    //Create the socket.
    int opt = true;
    auto fdMySock = socket(AF_UNIX, SOCK_STREAM, 0);
    if( fdMySock != INVALID_FD && (setsockopt(fdMySock, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<char *>(&opt), sizeof(opt) ) >= 0 ) )
    {
        if( bind(fdMySock, _2sockAddr(&_mySockAddr), sizeof(_mySockAddr)) >= 0 )
        {
            //                struct stat statRes;
            //                if(stat(_mySocketPath.c_str(), &statRes) >= 0)
            {
                //                    chmod(_mySocketPath.c_str(), umask(statRes.st_mode));
                // Max 5 pending connections
                if(listen(fdMySock, 5) == 0)
                {
                    mafInfo("Listening on address " << _myaddr.dump());
                    _stopped = false;
                    _listeningThread = std::thread{&LocalIPCReceiverImpl::listeningThreadFunc, this, fdMySock};
                    startable = true;
                }
                else
                {
                    mafErr("Could not listen on socket");
                }
            }
        }
        else
        {
            mafErr("Coud not bind socket to address " << _myaddr.dump());
        }
    }

    return startable;
}

bool LocalIPCReceiverImpl::stopListening()
{
    bool ret = false;

    if(listening())
    {
        _stopped = true;
        ret = true;
    }

    if(_listeningThread.joinable())
    {
        _listeningThread.join();
    }

    return ret;
}

bool LocalIPCReceiverImpl::listening() const
{
    return !_stopped.load(std::memory_order_acquire);
}

const Address &LocalIPCReceiverImpl::address() const
{
    return _myaddr;
}

bool LocalIPCReceiverImpl::listeningThreadFunc(int fdMySock)
{
    auto maxSd = INVALID_FD;
    socklen_t sockLen = sizeof(_mySockAddr);
    std::vector<int> fdClientSocks(MAXCLIENTS, 0);
    fd_set readfds;
    while(true)
    {
        //clear the socket set
        FD_ZERO(&readfds);

        //add master socket to set
        FD_SET(fdMySock, &readfds);
        maxSd = fdMySock;

        //add child sockets to set
        for ( size_t i = 0 ; i < MAXCLIENTS ; i++ )
        {
            //socket descriptor
            auto sd = fdClientSocks[i];

            //if valid socket descriptor then add to read list
            if(sd > 0)
                FD_SET( sd , &readfds);

            //highest file descriptor number, need it for the select function
            if(sd > maxSd)
                maxSd = sd;
        }

        timeval timeout = {1, 0};
        //wait for an activity on one of the sockets
        auto totalSD = select( maxSd + 1 , &readfds , nullptr , nullptr , &timeout);
        if(_stopped.load(std::memory_order_acquire))
        {
            mafInfo("Finish listening due to flag STOP was turned on, address: " << _myaddr.dump());
            break;
        }

        if ( totalSD <= 0)
        {
            continue;
        }

        //If something happened on the master socket ,
        //then its an incoming connection
        if (FD_ISSET(fdMySock, &readfds))
        {
            auto acceptedSD = accept(fdMySock, _2sockAddr(&_mySockAddr), &sockLen);
            if (acceptedSD < 0)
            {
                mafErr("Failed on accepting new socket connection");
                return false;
            }

            //add new socket to array of sockets
            for (size_t i = 0; i < MAXCLIENTS; i++)
            {
                //if position is empty
                if( fdClientSocks[i] == 0 )
                {
                    fdClientSocks[i] = acceptedSD;
                    break;
                }
            }
        }

        //else its some IO operation on some other socket
        for (size_t i = 0; i < MAXCLIENTS; i++)
        {
            auto sd = fdClientSocks[i];
            if (FD_ISSET( sd , &readfds))
            {
                SizeType messageLength = 0;
                bool failed = true;
                ssize_t totalRead = 0;
                if ( auto bytesRead = read( sd , reinterpret_cast<char*>(&messageLength), sizeof(SizeType));
                    bytesRead == sizeof(SizeType) )
                {
                    auto payload = std::make_shared<srz::ByteArray>(messageLength);
                    payload->resize(messageLength);
                    while(totalRead < messageLength)
                    {
                        if(bytesRead = read(sd, payload->data() + totalRead, messageLength);
                            bytesRead != -1)
                        {
                            totalRead += bytesRead;
                        }
                        else
                        {
                            mafUssErr("Could not read bytes from socket total read = " << totalRead << " total expected = " << messageLength);
                            break;
                        }
                    }
                    if (totalRead == messageLength)
                    {
                        notifyObervers(payload);
                        failed = false;
                    }
                }

                if(failed)
                {
                    //Somebody disconnected , get his details and print
//                    sockaddr_in peerAddr;
//                    getpeername(sd , _2sockAddr(&peerAddr), (socklen_t*)&addrlen);
//                     mafWarn("Disconnected from sender " << inet_ntoa(peerAddr.sin_addr)  << ntohs(peerAddr.sin_port));
                }
                close( sd );
                fdClientSocks[i] = 0;
            }
        }
    }

    _stopped = true;
    return true;
}

}
}
}

