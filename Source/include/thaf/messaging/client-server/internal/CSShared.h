#pragma once

#include "thaf/utils/cppextension/SyncObject.h"
#include "thaf/messaging/client-server/CSStatus.h"
#include "thaf/messaging/client-server/CSMessage.h"
#include <algorithm>


namespace thaf {
namespace messaging {

class ServiceMessageReceiver;

template<class Elem>
using SMElem = std::shared_ptr<Elem>;

template <class Elem>
using SMList = nstl::SyncObject<std::vector<SMElem<Elem>>>;

template<class Interester, std::enable_if_t<std::is_base_of_v<ServiceMessageReceiver, Interester>, bool> = true>
bool addIfNew(SMList<Interester>& interesters, SMElem<Interester> interester)
{
    bool added = false;
    auto lock(interesters.pa_lock());
    auto it = std::find(interesters->begin(), interesters->end(), interester);
    if(it == interesters->end())
    {
        interesters->emplace_back(interester);
        added = true;
    }
    return added;
}

template<class Interester, std::enable_if_t<std::is_base_of_v<ServiceMessageReceiver, Interester>, bool> = true>
bool remove(SMList<Interester>& interesters, SMElem<Interester> interester)
{
    return removeByID(interesters, interester->serviceID());
}

template<class Interester, std::enable_if_t<std::is_base_of_v<ServiceMessageReceiver, Interester>, bool> = true>
bool removeByID(SMList<Interester>& interesters, ServiceID sid)
{
    auto lock(interesters.pa_lock());
    for(auto it = interesters->begin(); it != interesters->end(); ++it)
    {
        if((*it)->serviceID() == sid)
        {
            interesters->erase(it);
            return true;
        }
    }
    return false;
}

template<class Interester, std::enable_if_t<std::is_base_of_v<ServiceMessageReceiver, Interester>, bool> = true>
SMElem<Interester> findByID(SMList<Interester>& interesters, ServiceID sid)
{
    auto lock(interesters.pa_lock());
    auto it = std::find_if(
        interesters->begin(), interesters->end(),
        [&sid](const auto& interester) { return (interester->serviceID() == sid); }
        );
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
    auto it = std::find_if(
        interesters->begin(), interesters->end(),
        [&sid](const auto& interester) { return (interester->serviceID() == sid); }
        );
    return (it != interesters->end());
}

}
}
