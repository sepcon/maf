#pragma once

#include <limits>
#include <streambuf>
#include <string>

namespace maf {
namespace srz {

template <typename Char, typename Traits = std::char_traits<Char>>
class BasicStreamBuf : public std::basic_streambuf<Char, Traits> {

private:
  typedef std::basic_streambuf<Char, Traits> Base;

public:
  typedef typename Base::char_type char_type;
  typedef typename Base::int_type int_type;
  typedef typename Base::pos_type pos_type;
  typedef typename Base::off_type off_type;
  typedef typename Base::traits_type traits_type;

  typedef typename std::basic_string<char_type> string_type;
  typedef typename std::basic_string_view<char_type> string_view_type;

public:
  BasicStreamBuf() = default;
  BasicStreamBuf(BasicStreamBuf &&) = default;
  BasicStreamBuf &operator=(BasicStreamBuf &&) = default;

  BasicStreamBuf(std::string &&str) : str_{std::move(str)} {}
  BasicStreamBuf(const std::string &str) : str_(str) {}

  const string_type &str() const { return str_; }
  string_type &str() { return str_; }
  void str(string_type &&s) { str_ = std::move(s); }
  void str(const string_type &s) { str_ = s; }

protected:
  virtual std::streamsize xsputn(const char_type *s, std::streamsize n);
  virtual int_type overflow(int_type);

protected:
  int_type eof() { return traits_type::eof(); }
  bool is_eof(int_type ch) { return ch == eof(); }

private:
  string_type str_;
};

// Put Area
// ========

template <typename Char, typename Traits>
std::streamsize BasicStreamBuf<Char, Traits>::xsputn(const char_type *s,
                                                     std::streamsize n) {
  str_.append(s, static_cast<size_t>(n));
  return n;
}

template <typename Char, typename Traits>
typename BasicStreamBuf<Char, Traits>::int_type
BasicStreamBuf<Char, Traits>::overflow(int_type ch) {
  if (is_eof(ch))
    return eof();
  else {
    char_type c = traits_type::to_char_type(ch);
    return static_cast<int_type>(xsputn(&c, 1));
  }
}

using namespace std;
template <class CharT, class Traits, class Alloc = std::allocator<CharT>>
class BasicStringBuf
    : public basic_streambuf<CharT, Traits> { // stream buffer maintaining an
                                              // allocated character array
public:
  typedef Alloc allocator_type;
  typedef basic_streambuf<CharT, Traits> Base;
  typedef basic_string<CharT, Traits, Alloc> string_type;
  typedef basic_string_view<CharT> string_view_type;
  typedef typename string_type::size_type size_type;

  explicit BasicStringBuf(ios_base::openmode _Mode = ios_base::in |
                                                     ios_base::out)
      : seekHigh_(nullptr), state_(getstate(_Mode)),
        alloc_() { // construct empty character buffer from mode
  }

  explicit BasicStringBuf(const string_type &str,
                          ios_base::openmode _Mode = ios_base::in |
                                                     ios_base::out)
      : alloc_(str.get_allocator()) { // construct character buffer from
                                      // string, mode
    init(str.c_str(), str.size(), getstate(_Mode));
  }

  BasicStringBuf(BasicStringBuf &&right) { // construct by moving _Right
    assignrv(move(right));
  }

  BasicStringBuf &operator=(BasicStringBuf &&_Right) { // assign from _Right
    assignrv(move(_Right));
    return (*this);
  }

  void assignrv(BasicStringBuf &&_Right) { // assign by moving _Right
    if (this != addressof(_Right)) {       // different, worth moving
      tidy();
      this->swap(_Right);
    }
  }

  void swap(BasicStringBuf &_Right) { // swap with _Right
    if (this != addressof(_Right)) {  // different, worth swapping
      Base::swap(_Right);
      swap(seekHigh_, _Right.seekHigh_);
      swap(state_, _Right.state_);
      _Swap_adl(alloc_, _Right.alloc_);
    }
  }

  BasicStringBuf(const BasicStringBuf &) = delete;
  BasicStringBuf &operator=(const BasicStringBuf &) = delete;

  virtual ~BasicStringBuf() noexcept { // destroy the object
    tidy();
  }

  enum {            // constants for bits in stream state
    Allocated_ = 1, // set if character array storage has been allocated,
                    // eback() points to allocated storage
    Constant_ = 2,  // set if character array nonmutable
    Noread_ = 4,    // set if character array cannot be read
    Append_ = 8,    // set if all writes are appends
    Atend_ = 16     // set if initial writes are appends
  };

  using int_type = typename Traits::int_type;
  using pos_type = typename Traits::pos_type;
  using off_type = typename Traits::off_type;

  string_view_type view() const {
    if (!(state_ & Constant_) &&
        Base::pptr() != nullptr) { // writable, make string from write buffer
      const auto base = Base::pbase();
      return string_view_type(
          base,
          static_cast<size_type>(_Max_value(Base::pptr(), seekHigh_) - base));
    } else if (!(state_ & Noread_) &&
               Base::gptr() !=
                   nullptr) { // readable, make string from read buffer
      const auto base = Base::eback();
      return string_view_type(base,
                              static_cast<size_type>(Base::egptr() - base));
    }
    return {};
  }

  [[nodiscard]] string_type
  str() const { // return string copy of character array
    string_type result(alloc_);
    if (!(state_ & Constant_) &&
        Base::pptr() != nullptr) { // writable, make string from write buffer
      const auto base = Base::pbase();
      result.assign(base, static_cast<size_type>(
                              _Max_value(Base::pptr(), seekHigh_) - base));
    } else if (!(state_ & Noread_) &&
               Base::gptr() !=
                   nullptr) { // readable, make string from read buffer
      const auto base = Base::eback();
      result.assign(base, static_cast<size_type>(Base::egptr() - base));
    }
    return (result);
  }

  void str(const string_type &_Newstr) { // replace character array from string
    tidy();
    init(_Newstr.c_str(), _Newstr.size(), state_);
  }

protected:
  virtual int_type
  overflow(int_type _Meta = Traits::eof()) { // put an element to stream
    if (state_ & Constant_) {
      return (Traits::eof()); // array nonmutable, fail
    }

    if (Traits::eq_int_type(Traits::eof(), _Meta)) {
      return (Traits::not_eof(_Meta)); // EOF, return success code
    }

    const auto _Pptr = Base::pptr();
    const auto _Epptr = Base::epptr();
    if (_Pptr != nullptr && _Pptr < _Epptr) { // room in buffer, store it
      *Base::_Pninc() = Traits::to_char_type(_Meta);
      seekHigh_ = _Pptr + 1;
      return (_Meta);
    }

    // grow buffer and store element
    size_t oldsize = 0;
    const auto _Oldptr = Base::eback();
    if (_Pptr != nullptr) {
      oldsize = static_cast<size_t>(_Epptr - _Oldptr);
    }

    size_t newsize;
    if (oldsize < MINSIZE_) {
      newsize = MINSIZE_;
    } else if (oldsize < INT_MAX / 2) { // grow by 50 percent
      newsize = oldsize << 1;
    } else if (oldsize < INT_MAX) {
      newsize = INT_MAX;
    } else { // buffer can't grow, fail
      return (Traits::eof());
    }

    const auto newptr = _Unfancy(alloc_.allocate(newsize));
    Traits::copy(newptr, _Oldptr, oldsize);

    const auto newpnext = newptr + oldsize;
    seekHigh_ = newpnext + 1; // to include _Meta

    Base::setp(newptr, newpnext, newptr + newsize);
    if (state_ & Noread_) { // maintain eback() == allocated pointer invariant
      Base::setg(newptr, nullptr, newptr);
    } else { // if readable, set the get area to initialized region
      Base::setg(newptr, newptr + (Base::gptr() - _Oldptr), seekHigh_);
    }

    if (state_ & Allocated_) {
      alloc_.deallocate(PointerTraits::pointer_to(*_Oldptr), oldsize);
    }

    state_ |= Allocated_;
    *Base::_Pninc() = Traits::to_char_type(_Meta);
    return (_Meta);
  }

  virtual int_type
  pbackfail(int_type _Meta = Traits::eof()) { // put an element back to stream
    const auto gptr = Base::gptr();
    if (gptr == nullptr || gptr <= Base::eback() ||
        (!Traits::eq_int_type(Traits::eof(), _Meta) &&
         !Traits::eq(Traits::to_char_type(_Meta), gptr[-1]) &&
         (state_ & Constant_))) { // can't put back, fail
      return (Traits::eof());
    }

    // back up one position and store put-back character
    Base::gbump(-1);
    if (!Traits::eq_int_type(Traits::eof(), _Meta)) {
      *Base::gptr() = Traits::to_char_type(_Meta);
    }

    return (Traits::not_eof(_Meta));
  }

  virtual int_type
  underflow() { // get an element from stream, but don't point past it
    const auto gptr = Base::gptr();
    if (gptr == nullptr) { // no character buffer, fail
      return (Traits::eof());
    }

    if (gptr < Base::egptr()) { // return buffered
      return (Traits::to_int_type(*gptr));
    }

    // try to add initialized characters from the put area into the get area
    const auto pptr = Base::pptr();
    if (!pptr || (state_ & Noread_)) { // no put area or read disallowed
      return (Traits::eof());
    }

    const auto _Local_highwater = _Max_value(seekHigh_, pptr);
    if (_Local_highwater <= gptr) { // nothing in the put area to take
      return (Traits::eof());
    }

    seekHigh_ = _Local_highwater;
    Base::setg(Base::eback(), Base::gptr(), _Local_highwater);
    return (Traits::to_int_type(*Base::gptr()));
  }

  virtual pos_type seekoff(
      off_type _Off, ios_base::seekdir _Way,
      ios_base::openmode mode =
          ios_base::in |
          ios_base::out) { // change position by _Off, according to _Way, _Mode
    const auto gptrold = Base::gptr();
    const auto ptrold = Base::pptr();
    if (ptrold != nullptr && seekHigh_ < ptrold) { // update high-water pointer
      seekHigh_ = ptrold;
    }

    const auto seeklow = Base::eback();
    const auto seekdist = seekHigh_ - seeklow;
    off_type newoff;
    switch (_Way) {
    case ios_base::beg:
      newoff = 0;
      break;
    case ios_base::end:
      newoff = seekdist;
      break;
    case ios_base::cur: {
      constexpr auto both = ios_base::in | ios_base::out;
      if ((mode & both) != both) { // prohibited by N4727 [stringbuf.virtuals]
                                   // Table 107 "seekoff positioning"
        if (mode & ios_base::in) {
          if (gptrold != nullptr || seeklow == nullptr) {
            newoff = gptrold - seeklow;
            break;
          }
        } else if ((mode & ios_base::out) &&
                   (ptrold != nullptr || seeklow == nullptr)) {
          newoff = ptrold - seeklow;
          break;
        }
      }
    }

      // fallthrough
    default:
      return (pos_type(off_type(-1)));
    }

    if (static_cast<unsigned long long>(_Off) + newoff >
        static_cast<unsigned long long>(seekdist)) {
      return (pos_type(off_type(-1)));
    }

    _Off += newoff;
    if (_Off != 0 && (((mode & ios_base::in) && gptrold == nullptr) ||
                      ((mode & ios_base::out) && ptrold == nullptr))) {
      return (pos_type(off_type(-1)));
    }

    const auto _Newptr = seeklow + _Off; // may perform nullptr + 0
    if ((mode & ios_base::in) && gptrold != nullptr) {
      Base::setg(seeklow, _Newptr, seekHigh_);
    }

    if ((mode & ios_base::out) && ptrold != nullptr) {
      Base::setp(seeklow, _Newptr, Base::epptr());
    }

    return (pos_type(_Off));
  }

  virtual pos_type
  seekpos(pos_type _Pos,
          ios_base::openmode _Mode =
              ios_base::in |
              ios_base::out) { // change position to _Pos, according to _Mode
    const auto _Off = static_cast<streamoff>(_Pos);
    const auto _Gptr_old = Base::gptr();
    const auto _Pptr_old = Base::pptr();
    if (_Pptr_old != nullptr &&
        seekHigh_ < _Pptr_old) { // update high-water pointer
      seekHigh_ = _Pptr_old;
    }

    const auto _Seeklow = Base::eback();
    const auto _Seekdist = seekHigh_ - _Seeklow;
    if (static_cast<unsigned long long>(_Off) >
        static_cast<unsigned long long>(_Seekdist)) {
      return (pos_type(off_type(-1)));
    }

    if (_Off != 0 && (((_Mode & ios_base::in) && _Gptr_old == nullptr) ||
                      ((_Mode & ios_base::out) && _Pptr_old == nullptr))) {
      return (pos_type(off_type(-1)));
    }

    const auto _Newptr = _Seeklow + _Off; // may perform nullptr + 0
    if ((_Mode & ios_base::in) && _Gptr_old != nullptr) {
      Base::setg(_Seeklow, _Newptr, seekHigh_);
    }

    if ((_Mode & ios_base::out) && _Pptr_old != nullptr) {
      Base::setp(_Seeklow, _Newptr, Base::epptr());
    }

    return (pos_type(_Off));
  }

  void
  init(const CharT *ptr, size_type count,
       int state) { // initialize buffer to [_Ptr, _Ptr + _Count), set state
    if (count >
        static_cast<size_type>(
            std::numeric_limits<int>::max())) { // TRANSITION, VSO#485517
      throw std::bad_alloc{};
    }

    if (count != 0 && (state & (Noread_ | Constant_)) !=
                          (Noread_ | Constant_)) { // finite buffer that can be
                                                   // read or written, set it up
      const auto pNew = alloc_.allocate(count);
      Traits::copy(pNew, ptr, count);
      seekHigh_ = pNew + count;

      if (!(state & Noread_)) {
        Base::setg(pNew, pNew, seekHigh_); // setup read buffer
      }

      if (!(state & Constant_)) { // setup write buffer, and maybe read buffer
        Base::setp(pNew, (state & (Atend_ | Append_)) ? seekHigh_ : pNew,
                   seekHigh_);

        if (state & Noread_) { // maintain "_Allocated == eback() points to
                               // buffer base" invariant
          Base::setg(pNew, nullptr, pNew);
        }
      }

      state |= Allocated_;
    } else {
      seekHigh_ = nullptr;
    }

    state_ = state;
  }

  void tidy() { // discard any allocated buffer and clear pointers
    if (state_ & Allocated_) {
      alloc_.deallocate(
          PointerTraits::pointer_to(*Base::eback()),
          static_cast<typename allocator_traits<allocator_type>::size_type>(
              (Base::pptr() != nullptr ? Base::epptr() : Base::egptr()) -
              Base::eback()));
    }

    Base::setg(nullptr, nullptr, nullptr);
    Base::setp(nullptr, nullptr);
    seekHigh_ = nullptr;
    state_ &= ~Allocated_;
  }

private:
  using PointerTraits =
      pointer_traits<typename allocator_traits<allocator_type>::pointer>;

  enum { // constant for minimum buffer size
    MINSIZE_ = 32
  };

  static int
  getstate(ios_base::openmode mode) { // convert open mode to stream state bits
    int state = 0;
    if (!(mode & ios_base::in)) {
      state |= Noread_;
    }

    if (!(mode & ios_base::out)) {
      state |= Constant_;
    }

    if (mode & ios_base::app) {
      state |= Append_;
    }

    if (mode & ios_base::ate) {
      state |= Atend_;
    }

    return (state);
  }

  CharT *seekHigh_;      // the high-water pointer in character array
  int state_;            // the stream state
  allocator_type alloc_; // the allocator object
};

} // namespace srz
} // namespace maf
