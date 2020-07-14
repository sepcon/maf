#pragma once

#include "ByteArray.h"
#include <cstring>

namespace maf {
namespace srz {

class OByteStream {
  using State = uint8_t;
  static constexpr State Good = 1;
  static constexpr State Failed = 2;

public:
  using SizeType = size_t;

  void write(const char *buf, SizeType size) {
    if (prepareNextWrite(size)) {
      std::memcpy(data_.data() + currentPos_, buf, size);
      currentPos_ += size;
    }
  }

  bool prepareNextWrite(SizeType size) {
    if (good()) {
      if (currentPos_ + size > data_.size()) {
        data_.resize(currentPos_ + size);
      }
      return true;
    }
    return false;
  }

  void reset() {
    currentPos_ = 0;
    state_ = Good;
    data_.clear();
  }

  bool good() const { return state_ & Good; }
  bool fail() const { return state_ & Failed; }
  ByteArray &bytes() { return data_; }
  const ByteArray &bytes() const { return data_; }

private:
  ByteArray data_;
  SizeType currentPos_ = 0;
  State state_ = Good;
};

namespace internal {

template <class StreamType, typename> struct StreamHelper;
template <> struct StreamHelper<OByteStream, void> {
  static void prepareNextWrite(OByteStream &obs, size_t nextWrittenCount) {
    obs.prepareNextWrite(nextWrittenCount);
  }
};

} // namespace internal

} // namespace srz
} // namespace maf
