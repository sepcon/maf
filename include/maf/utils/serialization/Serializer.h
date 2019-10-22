#pragma once

#include <utility>
#include <memory>
#include <ostream>
#include <istream>
#include "SerializationTrait.h"
#include "ByteArray.h"

namespace maf {
namespace srz {

class Serializer
{
public:
    template <typename SerializableObject, std::enable_if_t<std::is_class_v<SerializationTrait<SerializableObject>>, bool> = true>
    Serializer& operator<<(const SerializableObject& obj);
    Serializer& operator<<(const ByteArray& value);
    virtual void flush();
    virtual ~Serializer() = default;

protected:
    virtual char* getNextWriteArea(SizeType length) = 0;
    virtual void sync();
};

class Deserializer
{
public:
    template <typename T, std::enable_if_t<std::is_class_v<SerializationTrait<T>>, bool> = true>
    Deserializer& operator>>(T& obj);
    Deserializer& operator>>(ByteArray& obj);
    virtual bool exhausted() const = 0;
    virtual ~Deserializer() = default;

protected:
    virtual void fetchMoreBytes(const char** /*startp*/, const char** /*lastp*/, SizeType /*neededBytes*/);
    ByteArray::RBytePos _curpos = ByteArray::InvalidPos;
    ByteArray::RBytePos _lastpos = ByteArray::InvalidPos;
};

class BASerializer : public Serializer
{
public:
    char* getNextWriteArea(SizeType length) override;
    const ByteArray& bytes() const { return _ba; }
    ByteArray&& mutableBytes() { return std::move(_ba); }
    operator ByteArray() { return _ba; }
private:
    ByteArray _ba;
};


class BADeserializer : public Deserializer
{
public:
    BADeserializer(const ByteArray& stream);
    bool exhausted() const override;
    void reset();
private:
    const ByteArray* _cpByteArray;
};

class StreamSerializer : public Serializer
{
public:
    StreamSerializer(std::ostream& stream);
    char* getNextWriteArea(SizeType length) override;
    void sync() override;
    void flush() override;
    ~StreamSerializer() override;
private:
    void write(char* pstart, std::streamsize length);
    void writeSum();

    std::ostream& _ostream;
    std::streamsize _totalWrite;
    ByteArray _serializeBuffer;
};


class StreamDeserializer : public Deserializer
{
    static constexpr long long DEFAULT_BUFFER_SIZE = 500;
public:
    StreamDeserializer(std::istream& stream, SizeType buffSize = DEFAULT_BUFFER_SIZE);
    void resizeBuffer(SizeType size);
    bool exhausted() const override;

private:
    std::streamsize fillbuffer(ByteArray::WBytePos pos, std::streamsize length);
    constexpr std::streamsize streamsize(size_t val) { return static_cast<std::streamsize>( val ); }
    void fetchMoreBytes(const char** startp, const char** lastp, SizeType neededBytes) override;

    ByteArray _buffer;
    std::istream& _istream;
    std::streamsize _totalAvailableBytes;
    std::streamsize _totalRead;
};

template <typename SerializableObject, std::enable_if_t<std::is_class_v<SerializationTrait<SerializableObject>>, bool>>
Serializer& Serializer::operator<<(const SerializableObject& obj)
{
    auto valueSize = maf::srz::serializeSizeOf(obj);
    maf::srz::serialize(getNextWriteArea(valueSize), obj);
    sync();
    return *this;
}

template <typename T, std::enable_if_t<std::is_class_v<SerializationTrait<T>>, bool>>
Deserializer& Deserializer::operator>>(T& obj)
{
//    assert((_curpos != ByteArray::InvalidPos) && (_lastpos != ByteArray::InvalidPos));
    obj = maf::srz::deserialize<T>(&_curpos, &_lastpos, [this](const char** startp, const char** lastp, SizeType neededBytes){
        this->fetchMoreBytes(startp, lastp, neededBytes);
    });
    return *this;
}

}// srz
}// maf
