#ifndef IPCINFO_H
#define IPCINFO_H

#include "maf/utils/cppextension/SyncObject.h"
#include "maf/utils/IDManager.h"
#include "CSMessage.h"
#include "Address.h"
#include <future>
#include <map>
#include <list>
#include <set>

namespace maf {
namespace messaging {

struct RegID
{
    using RequestIDType = util::IDManager::IDType;
    static void allocateUniqueID(RegID& regid, util::IDManager& idmgr) { regid.requestID = idmgr.allocateNewID();}
    static void reclaimID(RegID regid, util::IDManager& idmgr) { idmgr.reclaimUsedID(regid.requestID); regid.requestID = util::IDManager::INVALID_ID;}
    bool valid() const { return util::IDManager::isValidID(requestID) && opID != OpIDInvalid; }

    RequestIDType requestID = util::IDManager::INVALID_ID;
    OpID opID = OpIDInvalid;
};

struct RegEntry
{
    RegEntry() = default;
    RegEntry(RegID::RequestIDType requestID, std::function<void (const std::shared_ptr<CSMessage>&) > callback)
        : requestID(requestID), callback(std::move(callback)){}

    RegID::RequestIDType requestID;
    std::function<void (const std::shared_ptr<CSMessage>&) > callback;
};

struct SyncRegEntry
{
    RegID::RequestIDType requestID;
    std::promise< std::shared_ptr<CSMessage> > _msgPromise;
};

using RegEntriesMap = nstl::SyncObject<std::map<OpID, std::list<RegEntry> >>;
using SyncRegEntriesMap = nstl::SyncObject<std::map<OpID, std::list<SyncRegEntry>>>;
using RegisteredClientsMap = nstl::SyncObject<std::map<Address, std::set<OpID>>>;


} // messaging
} // maf


#endif // IPCINFO_H
