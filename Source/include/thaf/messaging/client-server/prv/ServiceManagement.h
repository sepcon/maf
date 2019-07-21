#pragma once

#include "thaf/utils/cppextension/SyncObject.h"
#include "thaf/messaging/client-server/interfaces/CSStatus.h"
#include "thaf/messaging/client-server/interfaces/CSMessage.h"

namespace thaf {
namespace messaging {

class ServiceMessageReceiver;

template<class Elem>
using SMElem = std::shared_ptr<Elem>;

template <class Elem>
using SMList = stl::SyncObject<std::set<SMElem<Elem>, std::less<>>>;

template <class ServiceInterester, std::enable_if_t<std::is_base_of_v<ServiceMessageReceiver, ServiceInterester>, bool> = true>
bool operator<(const SMElem<ServiceInterester>& first, const SMElem<ServiceInterester>& second)
{
    return first->serviceID() < second->serviceID();
}
template <class ServiceInterester, std::enable_if_t<std::is_base_of_v<ServiceMessageReceiver, ServiceInterester>, bool> = true>
bool operator<(const SMElem<ServiceInterester>& first, const ServiceID& sid)
{
    return first->serviceID() < sid;
}
template <class ServiceInterester, std::enable_if_t<std::is_base_of_v<ServiceMessageReceiver, ServiceInterester>, bool> = true>
bool operator<(const ServiceID& sid, const SMElem<ServiceInterester>& second)
{
    return sid < second->serviceID();
}


template<class Interester, std::enable_if_t<std::is_base_of_v<ServiceMessageReceiver, Interester>, bool> = true>
bool addIfNew(SMList<Interester>& interesters, SMElem<Interester> interester)
{
    bool added = false;
    auto lock(interesters.pa_lock());
    auto it = interesters->find(interester);
    if(it == interesters->end())
    {
        interesters->insert(interester);
        added = true;
    }
    return added;
}

template<class Interester, std::enable_if_t<std::is_base_of_v<ServiceMessageReceiver, Interester>, bool> = true>
bool remove(SMList<Interester>& interesters, SMElem<Interester> interester)
{
    auto lock(interesters.pa_lock());
    return interesters->erase(interester) != 0;
}

template<class Interester, std::enable_if_t<std::is_base_of_v<ServiceMessageReceiver, Interester>, bool> = true>
bool removeByID(SMList<Interester>& interesters, ServiceID sid)
{
    auto lock(interesters.pa_lock());
    auto it = interesters->find(sid);
    if(it != interesters->end())
    {
        interesters->erase(it);
        return true;
    }
    else
    {
        return false;
    }
}
template<class Interester, std::enable_if_t<std::is_base_of_v<ServiceMessageReceiver, Interester>, bool> = true>
SMElem<Interester> findByID(SMList<Interester>& interesters, ServiceID sid)
{
    auto lock(interesters.pa_lock());
    auto it = interesters->find(sid);
    if(it != interesters->end())
    {
        return *it;
    }
    else
    {
        return {};
    }
}

template<class Interester, std::enable_if_t<std::is_base_of_v<ServiceMessageReceiver, Interester>, bool> = true>
bool hasItemWithID(SMList<Interester>& interesters, ServiceID sid)
{
    auto lock(interesters.pa_lock());
    auto it = interesters->find(sid);
    return (it != interesters->end());
}

}
}
