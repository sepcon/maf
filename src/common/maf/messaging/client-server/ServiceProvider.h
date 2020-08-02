#pragma once
#include <maf/messaging/client-server/ServiceProviderIF.h>
#include <maf/threading/Lockable.h>

#include <atomic>
#include <list>
#include <map>
#include <set>

namespace maf {
namespace messaging {

class ServerIF;
class Request;

class ServiceProvider : public ServiceProviderIF,
                        public std::enable_shared_from_this<ServiceProvider> {
  // clang-format off
  template <typename ValueType>
  using OpIDMap                = threading::Lockable<std::map<OpID, ValueType>>;

  using RequestPtr             = std::shared_ptr<Request>;
  using PropertyPtr            = CSPayloadIFPtr;
  using RequestMap             = OpIDMap<std::list<RequestPtr>>;
  using PropertyMap            = OpIDMap<PropertyPtr>;
  using RequestHandlerMap      = OpIDMap<RequestHandlerFunction>;
  using Address2OpIDsMap       = threading::Lockable<std::map<Address, std::set<OpID>>>;
  // clang-format on
 public:
  ServiceProvider(ServiceID sid, std::weak_ptr<ServerIF> server);

  ~ServiceProvider();

  const ServiceID &serviceID() const override;
  Availability availability() const override;

  ActionCallStatus respondToRequest(const CSMessagePtr &csMsg) override;

  CSPayloadIFPtr getStatus(const OpID &propertyID) override;
  ActionCallStatus setStatus(const OpID &propertyID,
                             const CSPayloadIFPtr &newProperty) override;

  ActionCallStatus broadcastSignal(const OpID &signalID,
                                   const CSPayloadIFPtr &signal) override;

  bool registerRequestHandler(const OpID &opID,
                              RequestHandlerFunction handlerFunction) override;
  bool unregisterRequestHandler(const OpID &opID) override;

  void startServing() override;
  void stopServing() override;

  bool onIncomingMessage(const CSMessagePtr &msg) override;

 private:
  ActionCallStatus broadcast(const OpID &propertyID, OpCode opCode,
                             const CSPayloadIFPtr &content);
  ActionCallStatus sendMessage(const CSMessagePtr &csMsg,
                               const Address &toAddr);
  ActionCallStatus sendBackMessageToClient(const CSMessagePtr &csMsg);
  void onStatusChangeRegister(const CSMessagePtr &msg);
  void onStatusChangeUnregister(const CSMessagePtr &msg);

  RequestPtr saveRequestInfo(const CSMessagePtr &msg);
  RequestPtr pickOutRequestInfo(const CSMessagePtr &msgContent);

  void invalidateAndRemoveAllRequests();

  void saveRegisterInfo(const CSMessagePtr &msg);
  void removeRegisterInfo(const CSMessagePtr &msg);
  void removeAllRegisterInfo();
  void removeRegistersOfAddress(const Address &addr);

  void onRequestAborted(const CSMessagePtr &msg);
  void onClientGoesOff(const CSMessagePtr &msg);

  void onActionRequest(const CSMessagePtr &msg);
  void updateLatestStatus(const CSMessagePtr &registerMsg);
  void onStatusGetRequest(const CSMessagePtr &getMsg);
  RequestHandlerFunction getRequestHandlerCallback(const OpID &opID);

 private:
  // clang-format off
  ServiceID                    sid_;
  Address2OpIDsMap             regEntriesMap_;
  RequestMap                   requestsMap_;
  std::weak_ptr<ServerIF>      server_;
  PropertyMap                  propertyMap_;
  RequestHandlerMap            requestHandlerMap_;
  std::atomic<Availability>    availability_ = Availability::Unavailable;
  // clang-format on
};

}  // namespace messaging
}  // namespace maf
