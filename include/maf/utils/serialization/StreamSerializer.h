#pragma once

#include "Serializer.h"
#include <maf/export/MafExport_global.h>

namespace maf {
namespace srz {

class StreamSerializer : public Serializer
{
public:
    MAF_EXPORT StreamSerializer(std::ostream& stream);
    MAF_EXPORT char* getNextWriteArea(SizeType length) override;
    MAF_EXPORT void sync() override;
    MAF_EXPORT void flush() override;
    MAF_EXPORT ~StreamSerializer() override;
private:
    void write(char* pstart, std::streamsize length);
    void writeSum();

    std::ostream&   _ostream;
    std::streamsize _totalWrite;
    ByteArray       _serializeBuffer;
};


class StreamDeserializer : public Deserializer
{
    static constexpr long long DEFAULT_BUFFER_SIZE = 500;
public:
    MAF_EXPORT StreamDeserializer(
        std::istream& stream,
        SizeType buffSize = DEFAULT_BUFFER_SIZE
        );

    MAF_EXPORT void resizeBuffer(SizeType size);
    MAF_EXPORT bool exhausted() const override;

private:
    std::streamsize fillbuffer(
        ByteArray::WBytePos pos,
        std::streamsize length
        );
    constexpr std::streamsize streamsize(size_t val)
    {
        return static_cast<std::streamsize>( val );
    }
    void fetchMoreBytes(
        const char** startp,
        const char** lastp,
        SizeType neededBytes
        ) override;

    ByteArray       _buffer;
    std::istream&   _istream;
    std::streamsize _totalAvailableBytes;
    std::streamsize _totalRead;
};


} // srz
} // maf
