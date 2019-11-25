
#include <maf/messaging/CompThread.h>

namespace maf {
namespace messaging {

CompThread &CompThread::start()
{
    auto compref = Component::getActiveWeakPtr();
    _callable =
        [
            callable = std::move(_callable),
            compref

    ] () mutable
    {
        Component::setTLRef(std::move(compref));
        callable();
    };
    Thread::start();
    return *this;
}


} //messaging
} //maf
