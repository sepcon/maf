#include <maf/utils/serialization/BASerializer.h>

namespace maf {
namespace srz {

char *BASerializer::getNextWriteArea(SizeType length)
{
    auto currentSize = _ba.size();
    _ba.resize(currentSize + length);
    return _ba.firstpos() + currentSize;
}

BADeserializer::BADeserializer(const ByteArray &stream) : _cpByteArray(&stream)
{
    _curpos = _cpByteArray->firstpos();
    _lastpos = _cpByteArray->lastpos();
}

bool BADeserializer::exhausted() const
{
    if(_cpByteArray)
        return _curpos > _cpByteArray->lastpos();
    else
        return true;
}

void BADeserializer::reset()
{
    if (_cpByteArray)
    {
        _curpos = _cpByteArray->firstpos();
    }
}

}
}
