#ifndef STREAM_H
#define STREAM_H

#include "ByteArray.h"
#include <cstring>

namespace maf {
namespace srz {

class IByteStream {
  using State = uint8_t;
  static constexpr State Good = 1;
  static constexpr State Failed = 2;

public:
  using SizeType = size_t;

  IByteStream(ByteArray &&ba) : data_{std::move(ba)} {}
  IByteStream(const ByteArray &ba) : data_{ba} {}

  void read(char *buf, SizeType size) {
    if (good()) {
      if (readingPos_ + size > data_.size()) {
        state_ &= Failed;
        return;
      }
      std::memcpy(buf, data_.data() + readingPos_, size);
      readingPos_ += size;
    }
  }

  bool good() const { return state_ & Good; }
  bool fail() const { return state_ & Failed; }
  ByteArray &bytes() { return data_; }
  const ByteArray &bytes() const { return data_; }

  void reset() {
    readingPos_ = 0;
    state_ = Good;
    data_.clear();
  }

private:
  ByteArray data_;
  SizeType readingPos_ = 0;
  State state_ = Good;
};

// template <typename Char, typename Traits = std::char_traits<Char>>
// class BasicIStringStream : public std::basic_istream<Char, Traits> {
// protected:
//  typedef std::basic_istream<Char, Traits> Base;

// public:
//  typedef typename Base::char_type char_type;
//  typedef typename Base::int_type int_type;
//  typedef typename Base::pos_type pos_type;
//  typedef typename Base::off_type off_type;
//  typedef typename Base::traits_type traits_type;
//  typedef
//      typename BasicStringBuf<Char, Traits>::string_type string_type;

// public:
//  BasicIStringStream() : Base(&buf_) {}
//  BasicIStringStream(BasicIStringStream&& right) : Base(&buf_),
//  buf_{std::move(right.buf_)} {} BasicIStringStream&
//  operator=(BasicIStringStream&&) = default; BasicIStringStream(const
//  string_type &str) : Base(&buf_), buf_{str} {} BasicIStringStream(string_type
//  &&str) : Base(&buf_), buf_{std::move(str)} {}

// public:
//  const string_type &str() const { return buf_.str(); }
//  string_type &str() { return buf_.str(); }

// private:
//  BasicStringBuf<Char, Traits> buf_;
//};

// using IByteStream = BasicIStringStream<char>;

} // namespace srz
} // namespace maf

#endif // STREAM_H
