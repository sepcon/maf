#pragma once

#include <utility>
#include <memory>
#include <ostream>
#include <istream>
#include "Serialization.h"
#include "ByteArray.h"

namespace thaf {
namespace srz {

class BASerializer
{
public:
    template <typename SerializableObject, std::enable_if_t<std::is_class_v<Serialization<SerializableObject>>, bool> = true>
    BASerializer& operator<<(const SerializableObject& obj)
    {
            using ObjectSerialization = Serialization<SerializableObject>;
            auto valueSize = ObjectSerialization::serializeSizeOf(obj);
            auto newSize = _ba.size() + valueSize;
            if (newSize >= _ba.capacity())
            {
                if (newSize < _ba.size() * 2) {
                    _ba.reserve(_ba.size() * 2);
                }
                else {
                    _ba.reserve(newSize * 2);
                }
            }
            auto occupiedCount = _ba.size();
            _ba.resize(newSize);
            ObjectSerialization::serialize(_ba.firstpos() + occupiedCount, obj);
            return *this;
    }
    const ByteArray& bytes() const
    {
        return _ba;
    }
    ByteArray& mutableBytes()
    {
        return _ba;
    }

private:
    ByteArray _ba;
};


class BADeserializer
{
public:
    BADeserializer() : _curPos(ByteArray::InvalidPos), _cpByteArray(nullptr) {}
    BADeserializer(const ByteArray& stream) : _curPos(stream.firstpos()), _cpByteArray(&stream) {}

    template <typename DeserializableObject, std::enable_if_t<std::is_class_v<Serialization<DeserializableObject>>, bool> = true>
    BADeserializer& operator>>(DeserializableObject& value)
    {
        if(_cpByteArray)
        {
            using ObjectDeserialization = Serialization<DeserializableObject>;
            auto lastPtr = _cpByteArray->lastpos();
            value = ObjectDeserialization::deserialize(&(this->_curPos), &lastPtr);
        }
        return *this;
    }

    void reset() 
	{
        if (_cpByteArray)
		{
            _curPos = _cpByteArray->data();
		}

    }

private:
    ByteArray::RBytePos _curPos;
    const ByteArray* _cpByteArray;
};



class StreamSerializer
{
public:
    StreamSerializer(std::ostream& stream) : _ostream(stream), _totalWrite(0)
    {
        writeSum(); //preserve first "sizeof(_totalWrite)" bytes for SUM
    }
    ~StreamSerializer()
    {
        flush();
    }

    template<typename T, std::enable_if_t<std::is_class_v<Serialization<T>>, bool> = true>
    StreamSerializer& operator<<(const T& value)
    {
        if(_ostream.good())
        {
            using TSerialization = Serialization<T>;
            auto valueSize = TSerialization::serializeSizeOf(value);
            ByteArray ba;
            ba.resize(valueSize);
            auto pstart = &ba[0];
            valueSize = TSerialization::serialize(pstart, value);
            write(pstart, valueSize);
        }
        return *this;
    }
    void flush()
    {
        if(_totalWrite > 0)
        {
            writeSum();
            _ostream.flush();
        }
    }

private:
    void write(char* pstart, std::streamsize length)
    {
        _ostream.write(pstart, length);
        _totalWrite += length;
    }
    void writeSum()
    {
        _ostream.seekp(std::streamoff{0}, std::ios_base::beg);
        _ostream.write(reinterpret_cast<char*>(&_totalWrite), sizeof(_totalWrite));
    }

    std::ostream& _ostream;
    std::streamsize _totalWrite;
};


class StreamDeserializer
{
    inline static constexpr long long DEFAULT_BUFFER_SIZE = 500;
public:
    StreamDeserializer(std::istream& stream, SizeType buffSize = DEFAULT_BUFFER_SIZE)
        : _istream(stream), _curpos(ByteArray::InvalidPos), _lastpos(ByteArray::InvalidPos), _totalAvailableBytes(0), _totalRead(0)
    {
        resizeBuffer(buffSize);
        _istream.read(reinterpret_cast<char*>(&_totalAvailableBytes), sizeof (_totalAvailableBytes));
    }



    template<typename T>
    StreamDeserializer& operator>>(T& obj)
    {
        using Srz = Serialization<T>;
		if (_totalRead <= 0 && _totalAvailableBytes > 0)
        {
            auto willbeRead = _totalAvailableBytes > streamsize(_buffer.size()) ? streamsize(_buffer.size()) : _totalAvailableBytes;
            _istream.read(_buffer.firstpos(), willbeRead);
            _totalRead = willbeRead;
			_curpos = _buffer.firstpos();
            _lastpos = _curpos + willbeRead - 1;
		}

        obj = Srz::deserialize(&_curpos, &_lastpos, [this](const char** startp, const char** lastp, SizeType neededBytes){
            this->fetchMoreBytes(startp, lastp, neededBytes);
        });
        return *this;
    }

    void resizeBuffer(SizeType size)
    {
        _buffer.resize(size);
    }

    std::streamsize fillbuffer(ByteArray::WBytePos pos, std::streamsize length)
    {
        std::streamsize read = 0;
        if(_totalRead < _totalAvailableBytes)
        {
            if(_totalRead + length <= _totalAvailableBytes)
            {
                _istream.read(pos, length);
                read = length;
            }
            else
            {
                read = _totalAvailableBytes - _totalRead;
                _istream.read(pos, read);
            }
            _totalRead += read;
        }
        return read;
    }

    bool exhausted() const
    {
        return _totalRead >= _totalAvailableBytes && _lastpos < _curpos;
    }

private:
    constexpr std::streamsize streamsize(size_t val) { return static_cast<std::streamsize>( val ); }
    void fetchMoreBytes(const char** startp, const char** lastp, SizeType neededBytes)
    {
        if(neededBytes > _buffer.size())
        {
            resizeBuffer(neededBytes);
        }
        std::streamoff seekOffset{0};
        std::streamsize remainerBytes = 0;

        if((*startp == *lastp) && (_totalRead > 0)) // all the buffer is consumed
        {
            remainerBytes = 1;
        }
        else if(*startp < *lastp)
        {
            remainerBytes = (*lastp - *startp + 1);
        }
        seekOffset = -remainerBytes;

        _totalRead -= static_cast<SizeType>(remainerBytes);
        _istream.seekg(_istream.tellg() + seekOffset);
        auto read = fillbuffer(_buffer.firstpos(), static_cast<std::streamsize>(_buffer.size()));
        *startp = _buffer.firstpos();
        *lastp = (*startp) + read - 1;
    }

    ByteArray _buffer;
    std::istream& _istream;
    ByteArray::RBytePos _curpos;
    ByteArray::RBytePos _lastpos;
    std::streamsize _totalAvailableBytes;
    std::streamsize _totalRead;
};
}// srz
}// thaf
