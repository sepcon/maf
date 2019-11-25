#pragma once

#include "Serializer.h"
#include <maf/export/MafExport_global.h>

namespace maf {
namespace srz {

class BASerializer : public Serializer
{
public:
    MAF_EXPORT char* getNextWriteArea(SizeType length) override;
    const ByteArray& bytes() const { return _ba; }
    ByteArray&& mutableBytes() { return std::move(_ba); }
    operator ByteArray&() { return _ba; }
    operator ByteArray&&() { return std::move(_ba); }
    operator const ByteArray&() { return _ba; }

private:
    ByteArray _ba;
};


class BADeserializer : public Deserializer
{
public:
    MAF_EXPORT BADeserializer(const ByteArray& stream);
    MAF_EXPORT bool exhausted() const override;
    MAF_EXPORT void reset();
private:
    const ByteArray* _cpByteArray;
};

} // srz
} // maf
