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
    template <typename SerializableObject>
    Serializer& operator<<(const SerializableObject& obj);
    Serializer& operator<<(const ByteArray& value)
    {
        return this->operator<<(static_cast<const std::string&>(value));
    }
    virtual void flush(){}
    virtual ~Serializer() = default;

protected:
    virtual char* getNextWriteArea(SizeType length) = 0;
    virtual void sync(){}
};

class Deserializer
{
public:
    template <typename T>
    Deserializer& operator>>(T& obj);
    Deserializer& operator>>(ByteArray& obj)
    {
        return this->operator>>(static_cast<std::string&>(obj));
    }

    virtual bool exhausted() const = 0;

protected:
    virtual ~Deserializer() = default;
    virtual void fetchMoreBytes(
        const char** /*startp*/,
        const char** /*lastp*/,
        SizeType /*neededBytes*/
        ){}

    ByteArray::RBytePos _curpos = ByteArray::InvalidPos;
    ByteArray::RBytePos _lastpos = ByteArray::InvalidPos;
};

template <typename T>
Serializer& Serializer::operator<<(const T& obj)
{
    static_assert (std::is_class_v<SerializationTrait<T>>, "");

    auto valueSize = maf::srz::serializeSizeOf(obj);
    maf::srz::serialize(getNextWriteArea(valueSize), obj);
    sync();
    return *this;
}

template <typename T>
Deserializer& Deserializer::operator>>(T& obj)
{
    static_assert (std::is_class_v<SerializationTrait<T>>, "");
    obj = maf::srz::deserialize<T>(&_curpos, &_lastpos, [this](const char** startp, const char** lastp, SizeType neededBytes){
        this->fetchMoreBytes(startp, lastp, neededBytes);
    });
    return *this;
}

}// srz
}// maf
