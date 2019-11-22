#pragma once

#include "Serializer.h"

namespace maf {
namespace srz {

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

} // srz
} // maf
