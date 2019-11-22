#ifndef IPCINFO_H
#define IPCINFO_H

#include <maf/utils/IDManager.h>
#include <maf/messaging/client-server/CSTypes.h>

namespace maf {
namespace messaging {

struct RegID
{
    using RequestIDType = util::IDManager::IDType;
    static void allocateUniqueID(RegID& regid, util::IDManager& idmgr)
    {
        regid.requestID = idmgr.allocateNewID();
    }
    static void reclaimID(const RegID& regid, util::IDManager& idmgr)
    {
        idmgr.reclaimUsedID(regid.requestID);
    }

    bool valid() const
    {
        return util::IDManager::isValidID(requestID) && opID != OpIDInvalid;
    }

    void clear()
    {
        requestID = util::IDManager::INVALID_ID;
        opID = OpIDInvalid;
    }

    RequestIDType requestID = util::IDManager::INVALID_ID;
    OpID opID               = OpIDInvalid;
};

} // messaging
} // maf


#endif // IPCINFO_H
