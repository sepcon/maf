#ifndef IPCINFO_H
#define IPCINFO_H

#include "thaf/Utils/CppExtension/AtomicContainer.h"
#include "thaf/Utils/IDManager.h"
#include "IPCMessage.h"
#include <future>

namespace thaf {
namespace messaging {
namespace ipc {

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
    RegEntry(RegID::RequestIDType requestID, std::function<void (const std::shared_ptr<IPCMessage>&) > callback)
        : requestID(requestID), callback(std::move(callback)){}

    RegID::RequestIDType requestID;
    std::function<void (const std::shared_ptr<IPCMessage>&) > callback;
};

struct SyncRegEntry
{
    RegID::RequestIDType requestID;
    std::promise< std::shared_ptr<IPCMessage> > _msgPromise;
};

using RegEntriesMap = stl::MutexContainer<std::map<OpID, std::list<RegEntry> >>;
using SyncRegEntriesMap = stl::MutexContainer<std::map<OpID, std::list<SyncRegEntry>>>;
using RegisteredClientsMap = stl::MutexContainer<std::map<Address, std::set<OpID>>>;


} // ipc
} // messaging
} // thaf


#endif // IPCINFO_H
