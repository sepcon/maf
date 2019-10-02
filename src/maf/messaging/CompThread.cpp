
#include <maf/messaging/CompThread.h>

namespace maf {
namespace messaging {

//CompThread::CompThread(CompThread &&th) : Thread{std::move(th) } {}

//CompThread &CompThread::operator=(CompThread &&th)
//{
//    static_cast<Thread&>(*this) = std::move(static_cast<Thread&>(th));
//    return *this;
//}

CompThread &CompThread::start()
{
    auto compref = Component::getActiveWeakPtr();
    _callable = [callableWrap = std::move(_callable), compref = std::move(compref)] () mutable {
        Component::setTLRef(std::move(compref));
        callableWrap();
    };
    Thread::start();
    return *this;
}


} //messaging
} //maf
