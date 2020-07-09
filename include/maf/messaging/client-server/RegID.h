#pragma once

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

  RegID() = default;
  ~RegID() {}  // provide this to avoid warning
  RegID(const RegID &) = default;
  RegID &operator=(const RegID &) = default;
  RegID(RegID &&other) noexcept
      : requestID(other.requestID), opID(std::move(other.opID)) {
    other.requestID = CSIDManager::INVALID_ID;
    other.opID = OpIDInvalid;
  }
  RegID &operator=(RegID &&other)  noexcept {
    if (&other != this) {
      requestID = other.requestID;
      opID = std::move(other.opID);
      other.requestID = CSIDManager::INVALID_ID;
      other.opID = OpIDInvalid;
    }
    return *this;
  }

  RequestIDType requestID = CSIDManager::INVALID_ID;
  OpID opID = OpIDInvalid;
};

inline bool operator==(const RegID &r1, const RegID &r2) {
  return (r1.requestID == r2.requestID) && (r1.opID == r2.opID);
}

inline bool operator!=(const RegID &r1, const RegID &r2) { return !(r1 == r2); }

inline bool operator<(const RegID &r1, const RegID &r2) {
  return (r1.requestID < r2.requestID) && (r1.opID < r2.opID);
}

}  // namespace messaging
}  // namespace maf
