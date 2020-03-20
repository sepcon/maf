#ifndef IDMANAGER_H
#define IDMANAGER_H

#ifdef RECLAIMABLE_ID_MANAGER
#include <mutex>
#include <set>

namespace maf {
namespace util {

class IDManager {
public:
  using IDType = unsigned int;
  static constexpr const IDType INVALID_ID = std::numeric_limits<IDType>::max();
  IDType allocateNewID();
  void reclaimUsedID(IDType id);
  static inline bool isValidID(IDType id) { return id != INVALID_ID; }

private:
  std::mutex mt_;
  std::set<IDType> inUsedIDSet_;
  std::set<IDType> fragmentedIDSet_;
};

} // namespace util
} // namespace maf

#else
#include <atomic>

namespace maf {
namespace util {
namespace details {

/**
 * @class IDManager is responsible for generating unique id, and reclaim the id
 * when it is no longer used.
 * @note: with the simple assumption that IDType is a large enough interger
 * values range that can not be overflow during life cycle of program. But
 * something wrong might happen in future, then new algorithm should be realized
 * to rework.
 */
template <typename IDType_> class IDManager {
public:
  using IDType = IDType_;
  static constexpr const IDType INVALID_ID = static_cast<IDType>(-1);
  IDManager() : idCounter_(0) {}
  IDType allocateNewID() {
    auto id = idCounter_.fetch_add(1, std::memory_order_relaxed);
    if (id == INVALID_ID) {
      id = idCounter_.fetch_add(1, std::memory_order_relaxed);
    }
    return id;
  }
  void reclaimUsedID(IDType /*id*/) {}
  static inline bool isValidID(IDType id) { return id != INVALID_ID; }

private:
  std::atomic<IDType> idCounter_;
};
} // namespace details

template <typename IDType> using IDManagerT = details::IDManager<IDType>;

using IDManager = IDManagerT<uint64_t>;

} // namespace util
} // namespace maf
#endif
#endif // IDMANAGER_H
