#ifndef IDMANAGER_H
#define IDMANAGER_H

#ifdef RECLAIMABLE_ID_MANAGER
#include <set>
#include <mutex>

namespace thaf {
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

namespace thaf {
namespace util {


class IDManager
{
public:
    using IDType = uint32_t;
    static constexpr const IDType INVALID_ID = static_cast<uint32_t>(-1);
    IDManager() : _idCounter(0){}
    IDType allocateNewID();
    void reclaimUsedID(IDType /*id*/) {}
    static inline bool isValidID(IDType id) { return id != INVALID_ID; }
private:
    std::atomic<IDType> _idCounter = 0;
};
}
}
#endif
#endif // IDMANAGER_H
