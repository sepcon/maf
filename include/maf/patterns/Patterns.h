#ifndef PATTERNS_H
#define PATTERNS_H

#include <memory>

namespace maf {
namespace pattern {


class UnMovable
{
protected:
    UnMovable(){}
    ~UnMovable(){}
private:
    UnMovable(UnMovable&&);
    UnMovable& operator=(UnMovable&);
};

class UnCopyable
{
protected:
    UnCopyable(){}
    ~UnCopyable(){}
private:
    UnCopyable(const UnCopyable&);
    UnCopyable& operator=(const UnCopyable&);
};


class Unasignable : public UnMovable, public UnCopyable
{
public:
	Unasignable() {}
	~Unasignable() {}
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
