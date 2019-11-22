#include "LocalIPCSenderImpl.h"
#include "SocketShared.h"

#define ns_global

namespace maf { using logging::Logger;
namespace messaging {
namespace ipc {

namespace
{
    AutoCloseFD<SockFD> connectToSocket(const std::string& sockpath)
    {
        AutoCloseFD<SockFD> fd;
        if(isValidSocketPath(sockpath))
        {
            if( fd = socket(AF_UNIX, SOCK_STREAM, 0); fd == INVALID_FD)
            {
                socketError("Cannot create socket on sockpath ", sockpath);
            }
            else
            {
                auto addr = createUnixAbstractSocketAddr(sockpath);
                if (connect(fd, _2sockAddr(&addr), sizeof(addr)) == INVALID_FD)
                {
                    socketError("Can't connect to address ", sockpath);
                    fd.reset();
                }
            }
        }
        else
        {
            Logger::error("Length of address exeeds the limitation of unix domain socket path");
        }

        return fd;
    }
}

LocalIPCSenderImpl::LocalIPCSenderImpl()
{
}

LocalIPCSenderImpl::~LocalIPCSenderImpl()
{
}

ActionCallStatus LocalIPCSenderImpl::send(const srz::ByteArray &payload, const Address &destination)
{
    ActionCallStatus ec = ActionCallStatus::FailedUnknown;
    SocketPath sockpath;

    if(destination.valid())
    {
        sockpath = constructSocketPath(destination);
    }
    else if(_myReceiverAddr && _myReceiverAddr->valid())
    {
        sockpath = *_myReceiverSocketPath;
    }


    if(auto fd = connectToSocket(sockpath); fd != INVALID_FD)
    {
        SizeType totalWritten = 0;
        SizeType payloadSize = static_cast<SizeType>(payload.length());

        // First write is to send payload size to receiver to reserve a buffer
        if(auto written = ns_global::write(fd, reinterpret_cast<char*>(&payloadSize), sizeof (SizeType));
                written == sizeof(SizeType))
        {
            // Second write is to send payload content
            do
            {
                if(written = ns_global::write(fd, payload.data() + totalWritten, payloadSize - totalWritten); written != -1)
                {
                    totalWritten += written;
                }
                else
                {
                    socketError("Failed to send bytes to receiver, total written = ", totalWritten);
                    ec = ActionCallStatus::FailedUnknown;
                    break;
                }
            }
            while(totalWritten < payloadSize);

            ec = ActionCallStatus::Success;
        }
        else
        {
            socketError("Failed to send payload size[", payloadSize, "] to receiver");
        }

        if(ec != ActionCallStatus::Success)
        {
            if(totalWritten == 0)
            {

            }
            else
            {
                //ec = FailedUnknown, must provide more info for debugging purpose
                Logger::error("Failed to send payload to receiver, expected is ", sizeof(SizeType), ", sent was ", totalWritten);
            }
        }
    }
    else
    {
        ec = ActionCallStatus::ReceiverUnavailable;
    }

    return ec;
}

bool LocalIPCSenderImpl::initConnection(const Address & receiverAddr)
{
    _myReceiverAddr = std::make_unique<Address>(receiverAddr);
    _myReceiverSocketPath = std::make_unique<SocketPath>(constructSocketPath(receiverAddr));
    return isValidSocketPath(*_myReceiverSocketPath);
}

Availability LocalIPCSenderImpl::checkReceiverStatus() const
{
    auto status = Availability::Unknown;
    if(_myReceiverSocketPath)
    {
        status = connectToSocket(*_myReceiverSocketPath) != INVALID_FD ? Availability::Available : Availability::Unavailable;
    }
    return status;
}

const Address &LocalIPCSenderImpl::receiverAddress() const
{
    if(_myReceiverAddr)
    {
        return *_myReceiverAddr;
    }
    else
    {
        return Address::INVALID_ADDRESS;
    }
}

}
}
}
