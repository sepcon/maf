#pragma once

#include <cstring>

#include "Buffer.h"

namespace maf {
namespace srz {

class IByteStream {
  using State = uint8_t;
  static constexpr State Good = 1;
  static constexpr State Failed = 2;
  static constexpr State Eof = 4;

 public:
  using SizeType = size_t;
  // NOTES: for purpose of IPC serialization in maf, IByteStream should be copy
  // constructible and assignable for storing state of current stream
  IByteStream(Buffer &&ba) noexcept : data_{std::move(ba)} {}
  IByteStream(const Buffer &ba) noexcept : data_{ba} {}
  IByteStream(const IByteStream &) = default;
  IByteStream &operator=(const IByteStream &) = default;

  IByteStream(IByteStream &&other) noexcept { other.moveTo(*this); }

  IByteStream &operator=(IByteStream &&other) noexcept {
    if (&other != this) {
      other.moveTo(*this);
    }
    return *this;
  }

  void read(char *buf, SizeType size) noexcept {
    if (good()) {
      if (readingPos_ + size > data_.size()) {
        state_ &= Failed;
        return;
      }
      std::memcpy(buf, data_.data() + readingPos_, size);
      readingPos_ += size;
      if (readingPos_ == data_.size()) {
        state_ &= Eof;
      }
    }
  }

  bool eof() const noexcept { return state_ & Eof; }
  bool good() const noexcept { return state_ & Good; }
  bool fail() const noexcept { return state_ & Failed; }
  Buffer &bytes() noexcept { return data_; }
  const Buffer &bytes() const noexcept { return data_; }

  void reset() noexcept {
    readingPos_ = 0;
    state_ = Good;
    data_.clear();
  }

  SizeType readingPos() const noexcept { return readingPos_; }

 private:
  void moveTo(IByteStream &other) noexcept {
    other.data_ = std::move(data_);
    other.readingPos_ = readingPos_;
    other.state_ = state_;
    state_ = Good;
    readingPos_ = 0;
  }
  Buffer data_;
  SizeType readingPos_ = 0;
  State state_ = Good;
};

}  // namespace srz
}  // namespace maf
