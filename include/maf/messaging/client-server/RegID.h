#ifndef IPCINFO_H
#define IPCINFO_H

#include <maf/messaging/client-server/CSTypes.h>
#include <maf/utils/IDManager.h>

namespace maf {
namespace messaging {

using CSIDManager = util::IDManagerT<RequestID>;

struct RegID {
  using RequestIDType = CSIDManager::IDType;
  static void allocateUniqueID(RegID &regid, CSIDManager &idmgr) {
    regid.requestID = idmgr.allocateNewID();
  }
  static void reclaimID(const RegID &regid, CSIDManager &idmgr) {
    idmgr.reclaimUsedID(regid.requestID);
  }

  bool valid() const {
    return CSIDManager::isValidID(requestID) && opID != OpIDInvalid;
  }

  void clear() {
    requestID = CSIDManager::INVALID_ID;
    opID = OpIDInvalid;
  }

  RequestIDType requestID = CSIDManager::INVALID_ID;
  OpID opID = OpIDInvalid;
};

} // namespace messaging
} // namespace maf

#endif // IPCINFO_H
