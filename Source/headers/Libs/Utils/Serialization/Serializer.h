#pragma once

#include <utility>
#include <memory>
#include "Serialization.h"
#include "ByteArray.h"

namespace thaf {
namespace srz {

class Serializer
{
public:
    template <typename SerializableObject, std::enable_if_t<std::is_class_v<Serialization<SerializableObject>>, bool> = true>
    Serializer& operator<<(const SerializableObject& obj)
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
            _ba.resize(newSize);
            ObjectSerialization::serialize(_ba.lastpos(), obj);
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


class Deserializer
{
public:
    Deserializer() : _curPos(ByteArray::InvalidPos), _cpByteArray(nullptr) {}
    Deserializer(const ByteArray& stream) : _curPos(stream.firstpos()), _cpByteArray(&stream) {}

    template <typename DeserializableObject, std::enable_if_t<std::is_class_v<Serialization<DeserializableObject>>, bool> = true>
    Deserializer& operator>>(DeserializableObject& value)
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


template <class OStream>
class StreamSerializer
{
public:
    StreamSerializer(OStream& stream) : _ostream(stream), _totalWrite(0)
    {
        writeSum(); //preserve first "sizeof(_totalWrite)" bytes for SUM
    }
    ~StreamSerializer()
    {
        close();
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
            TSerialization::serialize(pstart, value);
            write(pstart, valueSize);
        }
        return *this;
    }
    void close()
    {
        if(_totalWrite > 0)
        {
            writeSum();
            _totalWrite = 0;
        }
    }

private:
    void write(char* pstart, SizeType length)
    {
        _ostream.write(pstart, length);
        _totalWrite += length;
    }
    void writeSum()
    {
        _ostream.seekp(std::streamoff{0}, std::ios_base::beg);
        _ostream.write(reinterpret_cast<char*>(&_totalWrite), sizeof(_totalWrite));
    }

    OStream& _ostream;
    SizeType _totalWrite;
};

template <typename IStream>
class StreamDeserializer
{
    inline static constexpr long long DEFAULT_BUFFER_SIZE = 500;
public:
    StreamDeserializer(IStream& stream, long long buffSize = DEFAULT_BUFFER_SIZE)
        : _istream(stream), _totalAvailableBytes(0), _totalRead(0)
    {
        resizeBuffer(buffSize);
        _istream.read(reinterpret_cast<char*>(&_totalAvailableBytes), sizeof (_totalAvailableBytes));
        _curpos = _lastpos = _buffer.firstpos();
    }

    template<typename T>
    StreamDeserializer& operator>>(T& obj)
    {
        using Srz = Serialization<T>;
        obj = Srz::deserialize(&_curpos, &_lastpos, [this](const char** startp, const char** lastp, SizeType neededBytes){
            this->fetchMoreBytes(startp, lastp, neededBytes);
        });
        return *this;
    }

    void resizeBuffer(const long long size)
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
    bool exhaust() const
    {
        return _totalRead >= _totalAvailableBytes;
    }
private:
    void fetchMoreBytes(const char** startp, const char** lastp, SizeType neededBytes)
    {
        if(neededBytes > _buffer.size())
        {
            resizeBuffer(neededBytes);
        }
        std::streamoff seekOffset{0};
        int remainerBytes = 0;
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
        auto read = fillbuffer(_buffer.firstpos(), _buffer.size());
        *startp = _buffer.firstpos();
        *lastp = (*startp) + read - 1;
    }

    ByteArray _buffer;
    IStream& _istream;
    ByteArray::RBytePos _curpos;
    ByteArray::RBytePos _lastpos;
    SizeType _totalAvailableBytes;
    SizeType _totalRead;
};
}// srz
}// thaf
