#pragma once

#include <cstring>

#include "Buffer.h"

namespace maf {
namespace srz {
namespace details {
template <class Buff>
class BasicIByteStream {
 protected:
  using State = uint8_t;
  using BufferType = Buff;
  static constexpr State Good = 1;
  static constexpr State Failed = 2;
  static constexpr State Eof = 4;

 public:
  using SizeType = size_t;
  BasicIByteStream(BufferType buff, SizeType readingPos = 0, State state = Good)
      : buffer_(std::move(buff)), readingPos_{readingPos}, state_{state} {}

  BasicIByteStream(BasicIByteStream &&other) noexcept { other.moveTo(*this); }
  BasicIByteStream &operator=(BasicIByteStream &&other) noexcept {
    if (&other != this) {
      other.moveTo(*this);
    }
    return *this;
  }

  void read(char *buf, SizeType size) noexcept {
    if (good()) {
      if (readingPos_ + size > buffer_.size()) {
        (state_ &= ~Good) |= Failed;
        return;
      }
      std::memcpy(buf, buffer_.data() + readingPos_, size);
      readingPos_ += size;
      if (readingPos_ == buffer_.size()) {
        state_ |= Eof;
      }
    } else {
      (state_ &= ~Good) |= Failed;
    }
  }

  bool eof() const noexcept { return state_ & Eof; }
  bool good() const noexcept { return state_ & Good; }
  bool fail() const noexcept { return state_ & Failed; }
  void clear(State state) { state_ &= ~state; }

  void reset() noexcept {
    readingPos_ = 0;
    state_ = Good;
    buffer_.clear();
  }

  State state() const noexcept { return state_; }
  SizeType readingPos() const noexcept { return readingPos_; }

 protected:
  void moveTo(BasicIByteStream &other) noexcept {
    other.buffer_ = std::move(buffer_);
    other.readingPos_ = readingPos_;
    other.state_ = state_;
    state_ = Good;
    readingPos_ = 0;
  }
  BufferType buffer_;
  SizeType readingPos_ = 0;
  State state_ = Good;
};
}  // namespace details

class IByteStream : public details::BasicIByteStream<Buffer> {
 public:
  // NOTES: for purpose of IPC serialization in maf, BasicIByteStream should be
  // copy constructible and assignable for storing state of current stream
  using BasicIByteStream<Buffer>::BasicIByteStream;
  IByteStream(const IByteStream &other)
      : BasicIByteStream<Buffer>(other.buffer_, other.readingPos_,
                                 other.state_) {}
  Buffer &bytes() noexcept { return buffer_; }
  const Buffer &bytes() const noexcept { return buffer_; }
  decltype(auto) buffer() const { return bytes(); }

 private:
};

class IByteStreamView : public details::BasicIByteStream<const Buffer &> {
  using Base = BasicIByteStream<const Buffer &>;

 public:
  using Base::BasicIByteStream;
  IByteStreamView(const IByteStream &ibs)
      : BasicIByteStream<const Buffer &>(ibs.buffer(), ibs.readingPos(),
                                         ibs.state()) {}
};

}  // namespace srz
}  // namespace maf
