#include <maf/utils/serialization/StreamSerializer.h>

namespace maf {
namespace srz {

StreamSerializer::StreamSerializer(std::ostream &stream)
    : _ostream(stream), _totalWrite(0) {
  writeSum(); // preserve first "sizeof(_totalWrite)" bytes for SUM
}

StreamSerializer::~StreamSerializer() { flush(); }

char *StreamSerializer::getNextWriteArea(SizeType length) {
  if (_ostream.good()) {
    _serializeBuffer.resize(length);
    memset(_serializeBuffer.firstpos(), 0, length);
    return _serializeBuffer.firstpos();
  } else {
    throw _ostream.exceptions();
  }
}

void StreamSerializer::sync() {
  write(_serializeBuffer.firstpos(),
        static_cast<std::streamsize>(_serializeBuffer.size()));
}

void StreamSerializer::flush() {
  if (_totalWrite > 0) {
    writeSum();
    _ostream.flush();
  }
}

void StreamSerializer::write(char *pstart, std::streamsize length) {
  _ostream.write(pstart, length);
  _totalWrite += length;
}

void StreamSerializer::writeSum() {
  _ostream.seekp(std::streamoff{0}, std::ios_base::beg);
  _ostream.write(reinterpret_cast<char *>(&_totalWrite), sizeof(_totalWrite));
}

StreamDeserializer::StreamDeserializer(std::istream &stream, SizeType buffSize)
    : _istream(stream), _totalAvailableBytes(0), _totalRead(0) {
  resizeBuffer(buffSize);
  _istream.read(reinterpret_cast<char *>(&_totalAvailableBytes),
                sizeof(_totalAvailableBytes));
  if (_totalRead <= 0 && _totalAvailableBytes > 0) {
    auto willbeRead = _totalAvailableBytes > streamsize(_buffer.size())
                          ? streamsize(_buffer.size())
                          : _totalAvailableBytes;

    _istream.read(_buffer.firstpos(), willbeRead);
    _totalRead = willbeRead;
    _curpos = _buffer.firstpos();
    _lastpos = _curpos + willbeRead - 1;
  }
}

void StreamDeserializer::resizeBuffer(SizeType size) { _buffer.resize(size); }

std::streamsize StreamDeserializer::fillbuffer(ByteArray::WBytePos pos,
                                               std::streamsize length) {
  std::streamsize read = 0;
  if (_totalRead < _totalAvailableBytes) {
    if (_totalRead + length <= _totalAvailableBytes) {
      _istream.read(pos, length);
      read = length;
    } else {
      read = _totalAvailableBytes - _totalRead;
      _istream.read(pos, read);
    }
    _totalRead += read;
  }
  return read;
}

bool StreamDeserializer::exhausted() const {
  return _totalRead >= _totalAvailableBytes && _lastpos < _curpos;
}

void StreamDeserializer::fetchMoreBytes(const char **startp, const char **lastp,
                                        SizeType neededBytes) {
  if (neededBytes > _buffer.size()) {
    resizeBuffer(neededBytes);
  }
  std::streamoff seekOffset{0};
  std::streamsize remainerBytes = 0;

  if ((*startp == *lastp) && (_totalRead > 0)) // all the buffer is consumed
  {
    remainerBytes = 1;
  } else if (*startp < *lastp) {
    remainerBytes = (*lastp - *startp + 1);
  }
  seekOffset = -remainerBytes;

  _totalRead -= static_cast<SizeType>(remainerBytes);
  _istream.seekg(_istream.tellg() + seekOffset);
  auto read = fillbuffer(_buffer.firstpos(),
                         static_cast<std::streamsize>(_buffer.size()));
  *startp = _buffer.firstpos();
  *lastp = (*startp) + read - 1;
}

} // namespace srz
} // namespace maf
