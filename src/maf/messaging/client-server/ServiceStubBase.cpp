#include <maf/messaging/client-server/ServiceStubBase.h>
#include "ServiceStubBaseImpl.h"

namespace maf {
namespace messaging {

ServiceStubBase::ServiceStubBase(ServiceID sid, ServerInterface *server, ServiceStubHandlerInterface *stubHandler):
    _pImpl {new ServiceStubBaseImpl{server, stubHandler} } { setServiceID(sid);}
ServiceStubBase::~ServiceStubBase() { delete _pImpl; }
void ServiceStubBase::setStubHandler(ServiceStubHandlerInterface *stubHandler){ _pImpl->setStubHandler(stubHandler); }
bool ServiceStubBase::replyToRequest(const CSMessagePtr &csMsg, bool hasDone) {  return _pImpl->replyToRequest(csMsg, hasDone); }
bool ServiceStubBase::sendStatusUpdate(const CSMessagePtr &csMsg) { return _pImpl->sendStatusUpdate(csMsg); }
bool ServiceStubBase::onIncomingMessage(const CSMessagePtr &msg) { return _pImpl->onIncomingMessage(msg); }


} // messaging
} // maf
