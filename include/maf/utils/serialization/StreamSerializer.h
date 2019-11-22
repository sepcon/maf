#pragma once

#include "Serializer.h"

namespace maf {
namespace srz {

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


} // srz
} // maf
