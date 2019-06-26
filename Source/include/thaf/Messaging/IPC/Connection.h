#ifndef CONNECTION_H
#define CONNECTION_H

namespace thaf {
namespace messaging {
namespace ipc {

constexpr int WAIT_DURATION_MAX = 3000; // milliseconds
constexpr int BUFFER_SIZE = 1000;       // bytes

enum class ConnectionStatus : unsigned char
{
    Available,
    Busy,
    UnAvailable
};

enum class ConnectionErrorCode
{
    Success,
    Busy,
    Failed,
    Unknown
};

}// ipc
}// messaging
}// thaf
#endif // CONNECTION_H
