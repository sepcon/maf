#ifndef PATTERNS_H
#define PATTERNS_H

#include <memory>

namespace maf {
namespace pattern {


class UnMovable
{
public:
    UnMovable(){}
    ~UnMovable(){}
    UnMovable(UnMovable&&) = delete;
    UnMovable& operator=(UnMovable&&) = delete;
};

class UnCopyable
{
public:
    UnCopyable(){}
    ~UnCopyable(){}

    UnCopyable(const UnCopyable&) = delete;
    UnCopyable& operator=(const UnCopyable&) = delete;
};


class Unasignable
{
public:
	Unasignable() {}
	~Unasignable() {}
    Unasignable(Unasignable&&) = delete;
    Unasignable& operator=(Unasignable&&) = delete;
    Unasignable(const Unasignable&) = delete;
    Unasignable& operator=(const Unasignable&) = delete;
};

template <class ToBeSingleton>
class SingletonObject : public Unasignable
{
public:
    static ToBeSingleton& instance()
    {
        static ToBeSingleton __instance(Invisible{});
        return __instance;
    }

protected:
    struct Invisible{};
};

} // pattern
} // maf
#endif // PATTERNS_H
