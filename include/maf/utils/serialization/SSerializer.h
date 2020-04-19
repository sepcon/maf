#ifndef SSERIALIZER_H
#define SSERIALIZER_H

#include "Serializer1.h"
#include "ByteArray.h"
#include <sstream>

namespace maf {
namespace srz {

class SSerializer : public SR<std::ostringstream>
{
    using Base = SR<std::ostringstream>;
    std::ostringstream oss_;
public:
    SSerializer() : Base(oss_){}
    ByteArray bytes() const { return oss_.str(); }
    void reset() { return oss_.str(""); }
};


class SDeserializer : public DSR<std::istringstream>
{
    using Base = DSR<std::istringstream>;
    std::istringstream iss_;
public:
    SDeserializer(const ByteArray& ba) : Base(iss_){
        iss_.str(ba);
    }
};

}
}
#endif // SSERIALIZER_H
