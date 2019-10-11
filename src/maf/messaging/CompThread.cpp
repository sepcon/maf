
#include <maf/messaging/CompThread.h>

namespace maf {
namespace messaging {


CompThread &CompThread::start()
{
    _callable = [callableWrap = std::move(_callable), compref = Component::getActiveWeakPtr()] () mutable {
        Component::setTLRef(std::move(compref));
        callableWrap();
    };
    Thread::start();
    return *this;
}


} //messaging
} //maf
