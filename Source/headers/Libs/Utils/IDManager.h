#ifndef IDMANAGER_H
#define IDMANAGER_H

#include <set>
#include <limits>
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
#endif // IDMANAGER_H
