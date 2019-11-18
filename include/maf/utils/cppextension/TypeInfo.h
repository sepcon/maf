#pragma once
#ifndef NDEBUG

#include <typeinfo>
#include <cassert>
#endif

namespace maf {
namespace util {

template <class Type1, class Type2>
inline void debugAssertTypesEqual(Type1* t1, Type2* t2)
{
#ifndef NDEBUG
    if(t1 && t2)
    {
        assert(typeid(*t1) == typeid (*t2));
    }
#endif

}

} // util
} // maf
