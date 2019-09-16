#ifndef BYTEARRAY_H
#define BYTEARRAY_H

#include <string>

namespace maf {
namespace srz {

struct ByteArray : public std::string
{
    using               Byte                        = char;
    using               WBytePos                    = Byte*;
    using               RBytePos                    = const Byte*;
    static constexpr    WBytePos InvalidPos         = nullptr;
    using               std::string::string;
    ByteArray(const std::string& s) : std::string(s){}
    ByteArray(std::string&& s) : std::string(std::move(s)){}
    ByteArray(size_t capacity = 0)
    {
        reserve(capacity);
    }

    RBytePos firstpos() const
    {
        if(!empty())
        {
            return data();
        }
        else
        {
            return InvalidPos;
        }
    }

    RBytePos lastpos() const
    {
        if(!empty())
        {
            return &back();
        }
        else
        {
            return nullptr;
        }
    }

    WBytePos firstpos()
    {
        if(!empty())
        {
            return &operator[](0);
        }
        else
        {
            return InvalidPos;
        }
    }

    WBytePos lastpos()
    {
        if(!empty())
        {
            return &back();
        }
        else
        {
            return nullptr;
        }
    }
};

} // namespace srz
} // namespace maf

#endif // BYTEARRAY_H
