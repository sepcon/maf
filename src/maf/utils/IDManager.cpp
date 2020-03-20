#include <maf/utils/IDManager.h>

namespace maf {
namespace util {

#ifdef RECLAIMABLE_ID_MANAGER
IDManager::IDType IDManager::allocateNewID() {
  IDType id = INVALID_ID;
  std::lock_guard<std::mutex> lock(mt_);
  if (fragmentedIDSet_.empty()) {
    if (inUsedIDSet_.size() < INVALID_ID) {
      id = static_cast<IDType>(inUsedIDSet_.size());
      inUsedIDSet_.insert(static_cast<IDType>(inUsedIDSet_.size()));
    }
  } else {
    id = *fragmentedIDSet_.begin();
    fragmentedIDSet_.erase(fragmentedIDSet_.begin());
    inUsedIDSet_.insert(id);
  }
  return id;
}

void IDManager::reclaimUsedID(IDManager::IDType id) {
  std::lock_guard<std::mutex> lock(mt_);
  inUsedIDSet_.erase(id);
  fragmentedIDSet_.insert(std::move(id));
}
#else

#endif
} // namespace util
} // namespace maf
