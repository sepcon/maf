#ifndef IDMANAGER_H
#define IDMANAGER_H

#ifdef RECLAIMABLE_ID_MANAGER
#include <set>
#include <mutex>

namespace maf {
namespace util {


class IDManager
{
public:
    using IDType = unsigned int;
    static constexpr const IDType INVALID_ID = std::numeric_limits<IDType>::max();
    IDType allocateNewID();
    void reclaimUsedID(IDType id);
    static inline bool isValidID(IDType id) { return id != INVALID_ID; }
private:
    std::mutex _mt;
    std::set<IDType> _inUsedIDSet;
    std::set<IDType> _fragmentedIDSet;
};

}
}

#else
#include <atomic>

namespace maf {
namespace util {
namespace details {

/**
 * @class IDManager is responsible for generating unique id, and reclaim the id when
 * it is no longer used.
 * @note: with the simple assumption that IDType is a large enough interger
 * values range that can not be overflow during life cycle of program. But
 * something wrong might happen in future, then new algorithm should be realized
 * to rework.
 */
template<typename IDType_>
class IDManager
{
public:
    using IDType = IDType_;
    static constexpr const IDType INVALID_ID = static_cast<IDType>(-1);
    IDManager() : _idCounter(0){}
    IDType allocateNewID()
    {
        auto id = _idCounter.fetch_add(1, std::memory_order_relaxed);
        if(id == INVALID_ID)
        {
            id = _idCounter.fetch_add(1, std::memory_order_relaxed);
        }
        return id;
    }
    void reclaimUsedID(IDType /*id*/) {}
    static inline bool isValidID(IDType id) { return id != INVALID_ID; }
private:
    std::atomic<IDType> _idCounter;
};
}

template<typename IDType>
using IDManagerT = details::IDManager<IDType>;

using IDManager = IDManagerT<uint64_t>;


}
}
#endif
#endif // IDMANAGER_H
